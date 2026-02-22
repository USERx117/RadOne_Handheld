/**
  ******************************************************************************
  * @file    peripherals.c
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Peripherals Task
  *
  *          LED:     TIM1 CH1N/CH2N/CH3N on PB13/PB14/PB15 (common anode, N-channel)
  *          Buzzer:  TIM3 CH3 on PB0, PNP transistor (S8550) - inverted logic
  *          Battery: ADC1 CH14 on PB1 (1:2 resistor divider, VREF=3.3V)
  *          Buttons: PB5/PA8/PB10, GPIO Input, Pull-up, active low
  *
  *  LED behaviour:
  *    Idle:          weak white (20,20,20) - always on
  *    Tick < 0.5:    green  flash (50ms)
  *    Tick 0.5-2.0:  orange flash (50ms)
  *    Tick > 2.0:    red    flash (50ms)
  *    After 50ms:    back to weak white
  *    BatSave:       LED off
  *
  *  Tick handling:
  *    peripherals_on_tick() called from ISR - only sets flag.
  *    process_tick() runs in task context where ThreadX calls are safe.
  *    s_tick_active prevents idle white from overwriting the flash.
  ******************************************************************************
  */

#include "peripherals.h"
#include "main.h"
#include "tim.h"
#include "adc.h"
#include "gpio.h"
#include "power_mode.h"
#include "debug.h"

/* ==========================================================================
 * Dose rate thresholds for LED colour (uSv/h)
 * ========================================================================== */
#define DOSE_THRESHOLD_GREEN   0.5f
#define DOSE_THRESHOLD_ORANGE  2.0f

/* LED colours - TIM1 period=254, max value=254 */
#define LED_IDLE_R   20
#define LED_IDLE_G   20
#define LED_IDLE_B   20

#define LED_GREEN_R   0
#define LED_GREEN_G   200
#define LED_GREEN_B   0

#define LED_ORANGE_R  200
#define LED_ORANGE_G  80
#define LED_ORANGE_B  0

#define LED_RED_R     200
#define LED_RED_G     0
#define LED_RED_B     0

/* ==========================================================================
 * Button GPIO mapping
 * ========================================================================== */
#define BTN1_PIN    BTN_1_Pin
#define BTN1_PORT   BTN_1_GPIO_Port
#define BTN2_PIN    BTN_2_Pin
#define BTN2_PORT   BTN_2_GPIO_Port
#define BTN3_PIN    BTN_3_Pin
#define BTN3_PORT   BTN_3_GPIO_Port

/* ==========================================================================
 * Feedback timer and tick state
 * ========================================================================== */
static TX_TIMER          s_feedback_timer;
static volatile uint8_t  s_tick_pending = 0;   /* set in ISR, cleared in task  */
static volatile uint8_t  s_tick_active  = 0;   /* set in process_tick, cleared in callback */
static uint32_t          s_tick_counter = 0;
static volatile float    s_last_dose    = 0.0f;

static void feedback_timer_cb(ULONG arg)
{
    (void)arg;
    DEBUG_PRINT("FEEDBACK CB\r\n");
    s_tick_active = 0;
    /* Restore idle LED */
    if (power_is_led_active())
        led_set_color(LED_IDLE_R, LED_IDLE_G, LED_IDLE_B);
    else
        led_off();
    buzzer_off();
}

/* ==========================================================================
 * Button state machine
 * ========================================================================== */
typedef enum {
    BTN_STATE_IDLE = 0,
    BTN_STATE_DEBOUNCE,
    BTN_STATE_PRESSED,
    BTN_STATE_LONG,
} btn_state_t;

typedef struct {
    uint16_t         pin;
    GPIO_TypeDef    *port;
    btn_event_t      short_evt;
    btn_event_t      long_evt;
    btn_state_t      state;
    uint32_t         ticks;
} btn_ctx_t;

#define MS_TO_TICKS(ms)  ((ms) / BUTTON_POLL_MS)

static btn_ctx_t s_btns[3] = {
    { BTN1_PIN, BTN1_PORT, BTN_LEFT,  BTN_LEFT_LONG,  BTN_STATE_IDLE, 0 },
    { BTN2_PIN, BTN2_PORT, BTN_MID,   BTN_MID_LONG,   BTN_STATE_IDLE, 0 },
    { BTN3_PIN, BTN3_PORT, BTN_RIGHT, BTN_RIGHT_LONG, BTN_STATE_IDLE, 0 },
};

static void post_event(btn_event_t evt)
{
    DEBUG_PRINT("BTN POST evt=%d\r\n", (int)evt);
    tx_queue_send(&g_button_queue, &evt, TX_NO_WAIT);
}

static void button_poll(btn_ctx_t *btn)
{
    uint8_t pressed = (HAL_GPIO_ReadPin(btn->port, btn->pin) == GPIO_PIN_RESET);

    switch (btn->state)
    {
        case BTN_STATE_IDLE:
            if (pressed) {
                btn->state = BTN_STATE_DEBOUNCE;
                btn->ticks = 0;
            }
            break;

        case BTN_STATE_DEBOUNCE:
            if (!pressed) {
                btn->state = BTN_STATE_IDLE;
            } else if (++btn->ticks >= MS_TO_TICKS(BUTTON_DEBOUNCE_MS)) {
                btn->state = BTN_STATE_PRESSED;
                btn->ticks = 0;
            }
            break;

        case BTN_STATE_PRESSED:
            if (!pressed) {
                post_event(btn->short_evt);
                btn->state = BTN_STATE_IDLE;
            } else if (btn->long_evt != BTN_NONE &&
                       ++btn->ticks >= MS_TO_TICKS(BUTTON_LONG_MS)) {
                post_event(btn->long_evt);
                btn->state = BTN_STATE_LONG;
                btn->ticks = 0;
            }
            break;

        case BTN_STATE_LONG:
            if (!pressed) {
                btn->state = BTN_STATE_IDLE;
            } else if (++btn->ticks >= MS_TO_TICKS(BUTTON_REPEAT_MS)) {
                post_event(btn->long_evt);
                btn->ticks = 0;
            }
            break;
    }
}

/* ==========================================================================
 * LED colour based on dose rate
 * ========================================================================== */
static void led_tick_color(void)
{
    float dose = s_last_dose;
    if (dose < DOSE_THRESHOLD_GREEN)
        led_set_color(LED_GREEN_R,  LED_GREEN_G,  LED_GREEN_B);
    else if (dose < DOSE_THRESHOLD_ORANGE)
        led_set_color(LED_ORANGE_R, LED_ORANGE_G, LED_ORANGE_B);
    else
        led_set_color(LED_RED_R,    LED_RED_G,    LED_RED_B);
}

/* ==========================================================================
 * Process one sensor tick - task context only (ThreadX-safe)
 * ========================================================================== */
static void process_tick(void)
{
    s_tick_counter++;
    s_tick_active = 1;
    DEBUG_PRINT("TICK %lu mode=%d\r\n", s_tick_counter, (int)g_power_mode);

    if (g_power_mode == POWER_MODE_BUZZER_ONLY) {
        buzzer_on(BUZZER_FREQ_HZ);
    } else {
        if (power_is_led_active())
            led_tick_color();

        uint8_t divisor = power_buzzer_divisor();
        if (s_tick_counter % divisor == 0)
            buzzer_on(BUZZER_FREQ_HZ);
    }

    /* Reload and restart one-shot timer */
    tx_timer_deactivate(&s_feedback_timer);
    tx_timer_change(&s_feedback_timer, 5, 0);
    tx_timer_activate(&s_feedback_timer);
}

/* ==========================================================================
 * Public API
 * ========================================================================== */
void peripherals_update_dose(float dose_usv_h)
{
    s_last_dose = dose_usv_h;
}

void peripherals_task_init(void)
{
    /* RGB LED: complementary N-channel outputs on PB13/PB14/PB15 */
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
    __HAL_TIM_MOE_ENABLE(&htim1);

    /* Start at idle white */
    led_set_color(LED_IDLE_R, LED_IDLE_G, LED_IDLE_B);

    HAL_ADC_Start(&hadc1);
}

/* Called from ISR context - flag only, no ThreadX calls */
void peripherals_on_tick(void)
{
    s_tick_pending = 1;
}

void peripherals_task_entry(ULONG arg)
{
    (void)arg;

    /* Kernel is running - safe to create ThreadX timer here */
    tx_timer_create(&s_feedback_timer,
                    "FeedbackTimer",
                    feedback_timer_cb,
                    0,
                    5,      /* 5 ticks = 50ms at 100 ticks/s */
                    0,      /* one-shot */
                    TX_NO_ACTIVATE);

    /* Buzzer: start PWM once, duty=0 → pin HIGH → PNP off */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);

    while (1)
    {
        /* Idle LED - only when no tick flash is active */
        if (!s_tick_active) {
            if (power_is_led_active())
                led_set_color(LED_IDLE_R, LED_IDLE_G, LED_IDLE_B);
            else
                led_off();
        }

        /* Process pending sensor tick */
        if (s_tick_pending) {
            s_tick_pending = 0;
            process_tick();
        }

        /* Poll buttons */
        for (int i = 0; i < 3; i++)
            button_poll(&s_btns[i]);

        tx_thread_sleep(MS_TO_TICKS(BUTTON_POLL_MS));
    }
}

/* ==========================================================================
 * LED - TIM1 CH1N/CH2N/CH3N, common anode
 * ========================================================================== */
void led_set_color(uint8_t r, uint8_t g, uint8_t b)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, r);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, g);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, b);
}

void led_off(void) { led_set_color(0, 0, 0); }

/* ==========================================================================
 * Buzzer - TIM3 CH3, PNP (S8550), Polarity=Low
 * Duty=0   → pin HIGH → PNP off  → silent
 * Duty=50% → PWM      → PNP on   → sound
 * ========================================================================== */
void buzzer_on(uint16_t freq_hz)
{
    uint32_t arr = (1000000UL / freq_hz) - 1UL;
    __HAL_TIM_SET_AUTORELOAD(&htim3, arr);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, arr / 2);
}

void buzzer_off(void)
{
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
}

/* ==========================================================================
 * Battery
 * ========================================================================== */
float battery_get_voltage(void)
{
    uint32_t adc = HAL_ADC_GetValue(&hadc1);
    return ((float)adc / 4095.0f) * 3.3f * 2.0f;
}

uint8_t battery_get_percent(void)
{
    float v = battery_get_voltage();
    if (v >= BAT_VOLTAGE_MAX) return 100;
    if (v <= BAT_VOLTAGE_MIN) return 0;
    return (uint8_t)(((v - BAT_VOLTAGE_MIN) /
                      (BAT_VOLTAGE_MAX - BAT_VOLTAGE_MIN)) * 100.0f);
}
