/**
  ******************************************************************************
  * @file    peripherals.c
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Peripherals Task
  *
  *          LED:     TIM1 CH1/CH2/CH3 on PB13/PB14/PB15 (common anode)
  *          Buzzer:  TIM3 CH3 on PB0 (2kHz tone)
  *          Battery: ADC1 CH14 on PB1 (1:2 resistor divider, VREF=3.3V)
  *          Buttons: PB5/PA8/PB10, GPIO Input, Pull-up, active low
  ******************************************************************************
  */

#include "peripherals.h"
#include "main.h"
#include "tim.h"
#include "adc.h"
#include "gpio.h"
#include "usart.h"

/* ==========================================================================
 * Button GPIO mapping - uses CubeMX defines from main.h
 * BTN_1: PB5
 * BTN_2: PA8
 * BTN_3: PB10
 * ========================================================================== */
#define BTN1_PIN    BTN_1_Pin
#define BTN1_PORT   BTN_1_GPIO_Port
#define BTN2_PIN    BTN_2_Pin
#define BTN2_PORT   BTN_2_GPIO_Port
#define BTN3_PIN    BTN_3_Pin
#define BTN3_PORT   BTN_3_GPIO_Port

/* ==========================================================================
 * Private: feedback timer
 * ========================================================================== */
static TX_TIMER s_feedback_timer;

static void feedback_timer_cb(ULONG arg)
{
    (void)arg;
    led_off();
    buzzer_off();
}

/* ==========================================================================
 * Private: button state machine
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

/* ==========================================================================
 * Private: post event to button queue (non-blocking)
 * ========================================================================== */
static void post_event(btn_event_t evt)
{
    tx_queue_send(&g_button_queue, &evt, TX_NO_WAIT);
}

/* ==========================================================================
 * Private: poll one button, update state machine
 * ========================================================================== */
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
            } else if (++btn->ticks >= MS_TO_TICKS(BUTTON_LONG_MS)) {
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
 * Public API
 * ========================================================================== */
void peripherals_task_init(void)
{
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

    HAL_ADC_Start(&hadc1);

    tx_timer_create(&s_feedback_timer,
                    "FeedbackTimer",
                    feedback_timer_cb,
                    0,
                    TICK_FEEDBACK_MS,
                    0,
                    TX_NO_ACTIVATE);

    led_off();
    buzzer_off();
}

void peripherals_on_tick(void)
{
    led_set_color(0, 200, 0);
    buzzer_on(BUZZER_FREQ_HZ);
    tx_timer_deactivate(&s_feedback_timer);
    tx_timer_activate(&s_feedback_timer);
}

void peripherals_task_entry(ULONG arg)
{
    (void)arg;

    while (1)
    {
        for (int i = 0; i < 3; i++)
            button_poll(&s_btns[i]);

        tx_thread_sleep(MS_TO_TICKS(BUTTON_POLL_MS));
    }
}

/* ==========================================================================
 * LED - TIM1, ARR=254, common anode
 * ========================================================================== */
void led_set_color(uint8_t r, uint8_t g, uint8_t b)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, r);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, g);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, b);
}

void led_off(void) { led_set_color(0, 0, 0); }

/* ==========================================================================
 * Buzzer - TIM3 CH3, prescaler -> 1MHz, ARR = 1MHz/freq - 1
 * ========================================================================== */
void buzzer_on(uint16_t freq_hz)
{
    uint32_t arr = (1000000UL / freq_hz) - 1UL;
    __HAL_TIM_SET_AUTORELOAD(&htim3, arr);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, arr / 2);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
}

void buzzer_off(void) { HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3); }

/* ==========================================================================
 * Battery - ADC 12-bit, VREF=3.3V, 1:2 divider
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
