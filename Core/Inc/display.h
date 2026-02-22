/**
  ******************************************************************************
  * @file    display.h
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Display Task
  ******************************************************************************
  */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

#include "tx_api.h"
#include <stdint.h>
#include "app_threadx.h"

/* Task configuration */
#define DISPLAY_TASK_PRIORITY    6
#define DISPLAY_TASK_STACK_SIZE  4096

/* Task init and entry */
void display_task_init (TX_QUEUE *display_q, TX_QUEUE *button_q);
void display_task_entry(ULONG arg);

#endif /* INC_DISPLAY_H_ */
