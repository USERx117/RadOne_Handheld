/**
  ******************************************************************************
  * @file    power_mode.c
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Power Mode Management
  ******************************************************************************
  */

#include "power_mode.h"
#include "tim.h"
#include "debug.h"

/* ============================================================================
 * Global power mode
 * ============================================================================ */
volatile power_mode_t g_power_mode = POWER_MODE_DEFAULT;

/* ============================================================================
 * Set power mode - handles transitions
 * ============================================================================ */
void power_mode_set(power_mode_t mode)
{
    if (mode == g_power_mode) return;

    DEBUG_PRINT("POWER MODE -> %d\r\n", (int)mode);

    power_mode_t prev = g_power_mode;
    g_power_mode = mode;

    /* Handle display backlight */
    if (mode == POWER_MODE_BUZZER_ONLY) {
        /* Turn display off */
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
    } else if (prev == POWER_MODE_BUZZER_ONLY) {
        /* Restore default brightness when leaving buzzer-only */
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 800);
    }
}

/* ============================================================================
 * Get current mode
 * ============================================================================ */
power_mode_t power_mode_get(void)
{
    return g_power_mode;
}

/* ============================================================================
 * Query helpers
 * ============================================================================ */
uint8_t power_is_display_active(void)
{
    return (g_power_mode != POWER_MODE_BUZZER_ONLY);
}

uint8_t power_is_led_active(void)
{
    return (g_power_mode == POWER_MODE_DEFAULT ||
            g_power_mode == POWER_MODE_PERF);
}

uint8_t power_is_processing_active(void)
{
    return (g_power_mode != POWER_MODE_BUZZER_ONLY);
}

uint32_t power_display_refresh_ms(void)
{
    switch (g_power_mode) {
        case POWER_MODE_PERF:        return REFRESH_MS_PERF;
        case POWER_MODE_BATSAVE:     return REFRESH_MS_BATSAVE;
        case POWER_MODE_BUZZER_ONLY: return 0;
        default:                     return REFRESH_MS_DEFAULT;
    }
}

uint8_t power_buzzer_divisor(void)
{
    switch (g_power_mode) {
        case POWER_MODE_BATSAVE: return BUZZER_EVERY_BATSAVE;
        default:                 return BUZZER_EVERY_DEFAULT;
    }
}
