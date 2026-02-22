/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    gpio.c
 * @brief   This file provides code for the configuration
 *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
     PH0-OSC_IN (PH0)   ------> RCC_OSC_IN
     PH1-OSC_OUT (PH1)   ------> RCC_OSC_OUT
     PA13 (JTMS/SWDIO)   ------> DEBUG_JTMS-SWDIO
     PA14 (JTCK/SWCLK)   ------> DEBUG_JTCK-SWCLK
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, DISPLAY_CS_Pin|DISPLAY_DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DISPLAY_RES_GPIO_Port, DISPLAY_RES_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : DISPLAY_CS_Pin DISPLAY_DC_Pin */
  GPIO_InitStruct.Pin = DISPLAY_CS_Pin|DISPLAY_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : DISPLAY_RES_Pin */
  GPIO_InitStruct.Pin = DISPLAY_RES_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DISPLAY_RES_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : AL54_DIGITAL_IN_Pin */
  GPIO_InitStruct.Pin = AL54_DIGITAL_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(AL54_DIGITAL_IN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN_3_Pin BTN_1_Pin */
  GPIO_InitStruct.Pin = BTN_3_Pin|BTN_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : BTN_2_Pin */
  GPIO_InitStruct.Pin = BTN_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN_2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
