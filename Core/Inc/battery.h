/**
  ******************************************************************************
  * @file    battery.h
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Battery Monitor Task
  *
  *  Monitors battery voltage at runtime.
  *  Two thresholds:
  *    WARNING  (3.1V): Triggers low battery warning on display
  *    CRITICAL (2.9V): Full shutdown sequence → STM32 STANDBY mode
  *
  *  Task priority: 3 (highest in system - safety critical)
  ******************************************************************************
  */

#ifndef INC_BATTERY_H_
#define INC_BATTERY_H_

#include "tx_api.h"

/* ============================================================================
 * Task configuration
 * ============================================================================ */
#define BATTERY_TASK_PRIORITY       3
#define BATTERY_TASK_STACK_SIZE     512

#define BAT_CHECK_INTERVAL_MS       30000   /* Check every 30 seconds          */
#define BAT_WARNING_VOLTAGE         3.1f    /* Show warning below this voltage  */
#define BAT_CRITICAL_VOLTAGE_RT     2.9f    /* Shutdown below this voltage      */



/* ============================================================================
 * Public API
 * ============================================================================ */
void battery_task_init(void);
void battery_task_entry(ULONG arg);

#endif /* INC_BATTERY_H_ */
