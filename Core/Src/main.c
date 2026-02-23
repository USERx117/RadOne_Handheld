/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "app_threadx.h"
#include "main.h"
#include "adc.h"
#include "gpdma.h"
#include "icache.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "st7789.h"
#include "fonts.h"
#include "gfx_fonts.h"
#include "debug.h"
#include "battery.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BAT_CRITICAL_VOLTAGE   3.0f   /* V - below this: refuse to boot */
#define BAT_ADC_SAMPLES        8      /* average N samples for accuracy  */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static float battery_read_voltage(void);
static void  show_low_battery_screen(float voltage);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* ==========================================================================
 * Read battery voltage before kernel starts
 * ADC1 CH14 on PB1, 1:2 resistor divider, VREF=3.3V
 * ========================================================================== */
static float battery_read_voltage(void)
{
    uint32_t sum = 0;
    HAL_ADC_Start(&hadc1);
    for (int i = 0; i < BAT_ADC_SAMPLES; i++) {
        HAL_ADC_PollForConversion(&hadc1, 10);
        sum += HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);
    float avg = (float)sum / BAT_ADC_SAMPLES;
    return (avg / 4095.0f) * 3.3f * 2.0f;
}

/* ==========================================================================
 * Show low battery warning and halt
 * ========================================================================== */
static void show_low_battery_screen(float voltage)
{
#if RADONE_DEBUG
    (void)voltage;
    return;
#endif

    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 800);

    ST7789_InitBlocking();

    /* Warning symbol - red box with ! */
    ST7789_FillRect_blocking(90, 10, 60, 50, ST7789_RED);
    ST7789_DrawGFXString(111, 52, "!", ST7789_WHITE, ST7789_RED, &FreeSansBold24pt7b);

    /* LOW BATTERY */
    ST7789_DrawGFXString(10, 145, "LOW", ST7789_RED, ST7789_BLACK, &FreeSansBold24pt7b);
    ST7789_DrawGFXString(10, 185, "BATTERY", ST7789_RED, ST7789_BLACK, &FreeSansBold24pt7b);

    /* Voltage */
    char buf[16];
    snprintf(buf, sizeof(buf), "%.2fV", voltage);
    ST7789_DrawGFXString(55, 230, buf, ST7789_WHITE, ST7789_BLACK, &FreeSansBold18pt7b);

    /* Hint */
    ST7789_DrawString(22, 252, "Please charge (>3.0V)", ST7789_LIGHTGRAY, ST7789_BLACK, &Font_8x8);

    /* Halt forever - do not boot */
    while (1) {
        __WFI();
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_GPDMA1_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_USB_PCD_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_ICACHE_Init();

  /* USER CODE BEGIN 2 */

  /* ------------------------------------------------------------------
   * Battery check - must happen before ThreadX starts
   * If battery is critically low, show warning and halt.
   * ------------------------------------------------------------------ */
  float vbat = battery_read_voltage();
  if (vbat < BAT_CRITICAL_VOLTAGE) {
      show_low_battery_screen(vbat);
      /* never returns */
  }

  /* USER CODE END 2 */

  MX_ThreadX_Init();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  if (HAL_RCCEx_EpodBoosterClkConfig(RCC_EPODBOOSTER_SOURCE_MSIS, RCC_EPODBOOSTER_DIV1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_PWREx_EnableEpodBooster() != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_2);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSIS;
  RCC_OscInitStruct.MSISState = RCC_MSI_ON;
  RCC_OscInitStruct.MSISSource = RCC_MSI_RC0;
  RCC_OscInitStruct.MSISDiv = RCC_MSI_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSIS;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
