/**
  ******************************************************************************
  * @file    power_mode.h
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Power Mode Management
  *
  *  Power modes:
  *    DEFAULT:     Normal operation, 1s display refresh
  *    PERF:        Faster refresh (500ms), higher display priority
  *    BATSAVE:     5s display refresh, LED off, buzzer every 10th tick
  *    BUZZER_ONLY: Display off, processing off, direct tick → buzzer
  ******************************************************************************
  */

#ifndef INC_POWER_MODE_H_
#define INC_POWER_MODE_H_

#include <stdint.h>

/* ============================================================================
 * Power modes
 * ============================================================================ */
typedef enum {
    POWER_MODE_DEFAULT     = 0,
    POWER_MODE_PERF        = 1,
    POWER_MODE_BATSAVE     = 2,
    POWER_MODE_BUZZER_ONLY = 3,
} power_mode_t;

/* ============================================================================
 * Global power mode - read by all tasks
 * Defined in power_mode.c
 * ============================================================================ */
extern volatile power_mode_t g_power_mode;

/* ============================================================================
 * Display refresh intervals per mode (in ms)
 * ============================================================================ */
#define REFRESH_MS_DEFAULT      1000
#define REFRESH_MS_PERF          500
#define REFRESH_MS_BATSAVE      5000
#define REFRESH_MS_BUZZER_ONLY     0   /* display off */

/* ============================================================================
 * Buzzer tick divisor per mode
 * Buzzer fires every Nth sensor tick
 * ============================================================================ */
#define BUZZER_EVERY_DEFAULT    1    /* every tick  */
#define BUZZER_EVERY_BATSAVE    10   /* every 10th  */

/* ============================================================================
 * Public API
 * ============================================================================ */

/* Set a new power mode - handles transitions (display on/off, LED, etc.) */
void power_mode_set(power_mode_t mode);

/* Get current mode */
power_mode_t power_mode_get(void);

/* Query helpers - use these in tasks instead of reading g_power_mode directly */
uint8_t power_is_display_active(void);   /* 0 in BUZZER_ONLY */
uint8_t power_is_led_active(void);       /* 0 in BATSAVE and BUZZER_ONLY */
uint8_t power_is_processing_active(void);/* 0 in BUZZER_ONLY */
uint32_t power_display_refresh_ms(void); /* refresh interval for current mode */
uint8_t power_buzzer_divisor(void);      /* every Nth tick fires buzzer */

#endif /* INC_POWER_MODE_H_ */
