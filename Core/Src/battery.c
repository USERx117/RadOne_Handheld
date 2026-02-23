/**
  ******************************************************************************
  * @file    battery.c
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Battery Monitor Task
  *
  *  Priority 3 - highest in system. Checks battery voltage every 30s.
  *  WARNING  at 3.1V: warning screen (or buzzer signal in BUZZER_ONLY mode)
  *  CRITICAL at 2.9V: full shutdown → STM32 STANDBY mode
  *
  *  Power mode awareness:
  *    BUZZER_ONLY: no display interaction, buzzer warning signal instead
  *    All others:  full warning/shutdown screen shown
  ******************************************************************************
  */

#include "battery.h"
#include "peripherals.h"
#include "st7789.h"
#include "fonts.h"
#include "gfx_fonts.h"
#include "tim.h"
#include "adc.h"
#include "power_mode.h"
#include "debug.h"
#include "stm32u3xx_hal.h"

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;

/* ============================================================================
 * Private helpers
 * ============================================================================ */

static float bat_read_voltage(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint32_t adc = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return ((float)adc / 4095.0f) * 3.3f * 2.0f;
}

/* --------------------------------------------------------------------------
 * Three short buzzer beeps - used as warning signal in BUZZER_ONLY mode
 * -------------------------------------------------------------------------- */
static void bat_buzzer_warning(void)
{
    for (int i = 0; i < 3; i++) {
        buzzer_on(500);         /* low frequency = warning tone */
        tx_thread_sleep(50);    /* 500ms on  */
        buzzer_off();
        tx_thread_sleep(50);    /* 500ms off */
    }
}

/* --------------------------------------------------------------------------
 * Full shutdown sequence - always runs regardless of power mode.
 * Shows screen if display is active, then enters STM32 STANDBY.
 * -------------------------------------------------------------------------- */
static void battery_shutdown(float voltage)
{
    DEBUG_PRINT("BAT CRITICAL %.2f V - SHUTDOWN\r\n", (double)voltage);

    /* 1. Turn off LED and buzzer */
    led_off();
    buzzer_off();

    /* 2. Show critical screen only if display is active */
    if (g_power_mode != POWER_MODE_BUZZER_ONLY) {
        ST7789_FillScreen(ST7789_BLACK);
        ST7789_FillRect(90, 10, 60, 50, ST7789_RED);
        ST7789_DrawGFXString(111, 52, "!", ST7789_WHITE, ST7789_RED, &FreeSansBold24pt7b);
        ST7789_DrawGFXString(10, 145, "LOW", ST7789_RED, ST7789_BLACK, &FreeSansBold24pt7b);
        ST7789_DrawGFXString(10, 185, "BATTERY", ST7789_RED, ST7789_BLACK, &FreeSansBold24pt7b);

        char buf[16];
        snprintf(buf, sizeof(buf), "%.2fV", voltage);
        ST7789_DrawGFXString(30, 230, buf, ST7789_WHITE, ST7789_BLACK, &FreeSansBold18pt7b);
        ST7789_DrawString(22, 252, "Please charge (>3.0V)", ST7789_LIGHTGRAY, ST7789_BLACK, &Font_8x8);

        /* Show for 5 seconds so user can read it */
        tx_thread_sleep(500);

        /* Blank display and put it to sleep */
        ST7789_FillScreen(ST7789_BLACK);
        ST7789_Sleep();

        /* Backlight off */
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
    } else {
        /* BUZZER_ONLY: no display - just beep three times then go dark */
        bat_buzzer_warning();
        buzzer_off();
    }

    /* Enter STM32 STANDBY - lowest power, ~300nA */
    HAL_PWR_DisableWakeUpLine(PWR_WAKEUP_LINE1);
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SBF);
    HAL_PWR_EnterSTANDBYMode();

    /* Should never reach here */
    while (1) { __WFI(); }
}

/* --------------------------------------------------------------------------
 * Warning - shown at 3.1V, system continues running after 5s
 * -------------------------------------------------------------------------- */
static void battery_show_warning(float voltage)
{
    DEBUG_PRINT("BAT WARNING %.2f V\r\n", (double)voltage);

    if (g_power_mode == POWER_MODE_BUZZER_ONLY) {
        /* No display in buzzer-only mode - use buzzer signal */
        bat_buzzer_warning();
        return;
    }

    /* Display warning screen for 5 seconds, then display_task redraws */
    ST7789_FillScreen(ST7789_BLACK);
    ST7789_DrawGFXString(10, 80,  "LOW",     ST7789_ORANGE, ST7789_BLACK, &FreeSansBold24pt7b);
    ST7789_DrawGFXString(10, 120, "BATTERY", ST7789_ORANGE, ST7789_BLACK, &FreeSansBold24pt7b);

    char buf[16];
    snprintf(buf, sizeof(buf), "%.2fV", voltage);
    ST7789_DrawGFXString(30, 170, buf, ST7789_WHITE, ST7789_BLACK, &FreeSansBold18pt7b);
    ST7789_DrawString(10, 200, "Please charge soon!", ST7789_LIGHTGRAY, ST7789_BLACK, &Font_8x8);

    tx_thread_sleep(500);
}

/* ============================================================================
 * Task entry
 * ============================================================================ */
void battery_task_init(void)
{
    /* Nothing to init - ADC already started in peripherals_task_init */
}

void battery_task_entry(ULONG arg)
{
    (void)arg;

    /* Initial delay - let system fully boot before first check */
#if BATTERY_TASK_TEST
    tx_thread_sleep(200);   /* 2 seconds in test mode */
#else
    tx_thread_sleep(1000);  /* 10 seconds normal      */
#endif

    while (1)
    {
#if BATTERY_TASK_TEST
        float v = BAT_TEST_VOLTAGE;
        DEBUG_PRINT("BAT TEST MODE: %.2f V\r\n", (double)v);
#else
        float v = bat_read_voltage();
        DEBUG_PRINT("BAT %.2f V\r\n", (double)v);
#endif

        if (v < BAT_CRITICAL_VOLTAGE_RT) {
            battery_shutdown(v);
            /* never returns */
        } else if (v < BAT_WARNING_VOLTAGE) {
            battery_show_warning(v);
        }

        /* Sleep until next check */
#if BATTERY_TASK_TEST
        tx_thread_sleep(200);                        /* 2s in test mode */
#else
        tx_thread_sleep(BAT_CHECK_INTERVAL_MS / 10); /* 30s normal      */
#endif
    }
}
