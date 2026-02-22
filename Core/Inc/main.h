/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u3xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DISPLAY_BL_Pin GPIO_PIN_1
#define DISPLAY_BL_GPIO_Port GPIOA
#define DISPLAY_CS_Pin GPIO_PIN_2
#define DISPLAY_CS_GPIO_Port GPIOA
#define DISPLAY_DC_Pin GPIO_PIN_3
#define DISPLAY_DC_GPIO_Port GPIOA
#define DISPLAY_RES_Pin GPIO_PIN_4
#define DISPLAY_RES_GPIO_Port GPIOA
#define BUZZER_PWM_Pin GPIO_PIN_0
#define BUZZER_PWM_GPIO_Port GPIOB
#define BAT_CHECK_Pin GPIO_PIN_1
#define BAT_CHECK_GPIO_Port GPIOB
#define AL54_DIGITAL_IN_Pin GPIO_PIN_2
#define AL54_DIGITAL_IN_GPIO_Port GPIOB
#define AL54_DIGITAL_IN_EXTI_IRQn EXTI2_IRQn
#define BTN_3_Pin GPIO_PIN_10
#define BTN_3_GPIO_Port GPIOB
#define RGB1_Pin GPIO_PIN_13
#define RGB1_GPIO_Port GPIOB
#define RGB2_Pin GPIO_PIN_14
#define RGB2_GPIO_Port GPIOB
#define RGB3_Pin GPIO_PIN_15
#define RGB3_GPIO_Port GPIOB
#define BTN_2_Pin GPIO_PIN_8
#define BTN_2_GPIO_Port GPIOA
#define BTN_1_Pin GPIO_PIN_5
#define BTN_1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
