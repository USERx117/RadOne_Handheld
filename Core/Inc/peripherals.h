/**
  ******************************************************************************
  * @file    peripherals.h
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Peripherals Task
  ******************************************************************************
  */

#ifndef INC_PERIPHERALS_H_
#define INC_PERIPHERALS_H_

#include "tx_api.h"
#include <stdint.h>

/* ==========================================================================
 * Task configuration
 * ========================================================================== */
#define PERIPHERALS_TASK_PRIORITY    10
#define PERIPHERALS_TASK_STACK_SIZE  1024

/* ==========================================================================
 * Button timing (all in ms)
 * ========================================================================== */
#define BUTTON_POLL_MS      20    /* Polling interval                        */
#define BUTTON_DEBOUNCE_MS  40    /* Must be stable for this long            */
#define BUTTON_LONG_MS      500   /* Hold time before long-press fires       */
#define BUTTON_REPEAT_MS    150   /* Repeat interval while held (brightness) */
#define BUTTON_COOLDOWN_MS  100   /* Ignore input after short press          */

/* ==========================================================================
 * Button events - posted to g_button_queue
 * ========================================================================== */
typedef enum {
    BTN_NONE       = 0,
    BTN_LEFT,           /* Short press: navigate left  */
    BTN_MID,            /* Short press: select / enter */
    BTN_RIGHT,          /* Short press: navigate right */
    BTN_LEFT_LONG,      /* Long press:  brightness -        */
    BTN_MID_LONG,       /* Long press:  exit Buzzer-Only    */
    BTN_RIGHT_LONG,     /* Long press:  brightness +        */
} btn_event_t;

/* ==========================================================================
 * Button queue - defined in app_threadx.c, used here and in display task
 * ========================================================================== */
extern TX_QUEUE g_button_queue;

/* ==========================================================================
 * Peripherals constants
 * ========================================================================== */
#define TICK_FEEDBACK_MS    50
#define BUZZER_FREQ_HZ      2000
#define BAT_VOLTAGE_MAX     4.2f
#define BAT_VOLTAGE_MIN     3.0f

/* ==========================================================================
 * Public API
 * ========================================================================== */
void peripherals_task_init(void);
void peripherals_task_entry(ULONG arg);
void peripherals_on_tick(void);
void peripherals_update_dose(float dose_usv_h);

void    led_set_color(uint8_t r, uint8_t g, uint8_t b);
void    led_off(void);
void    buzzer_on(uint16_t freq_hz);
void    buzzer_off(void);
float   battery_get_voltage(void);
uint8_t battery_get_percent(void);

#endif /* INC_PERIPHERALS_H_ */
