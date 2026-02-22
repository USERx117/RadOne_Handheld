/**
  ******************************************************************************
  * @file    processing.h
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Processing Task
  *
  *          Responsibilities:
  *          - Receive sensor_data_t from g_processing_queue (1x per second)
  *          - Calculate CPM from 10s, 30s, 60s tick windows
  *          - Convert CPM to uSv/h using AL54 calibration factor
  *          - Apply EWMA smoothing to 10s and 30s windows
  *          - Post display_data_t snapshot to g_display_queue
  *          - Print debug status to UART every 10 seconds
  *
  *          AL54 calibration: Dose (uSv/h) = CPM / 5
  *          (Datasheet: 5 cpm per uSv/h, +/-15% tolerance)
  ******************************************************************************
  */

#ifndef INC_PROCESSING_H_
#define INC_PROCESSING_H_

#include "app_threadx.h"

/* Task configuration */
#define PROCESSING_TASK_PRIORITY    8
#define PROCESSING_TASK_STACK_SIZE  2048

/* AL54 conversion: 5 CPM = 1 uSv/h  ->  1 CPM = 0.2 uSv/h */
#define AL54_CPM_TO_USV_H       0.2f

/* EWMA smoothing factors (alpha: 0=frozen, 1=no smoothing) */
#define EWMA_ALPHA_10S          0.3f    /* More responsive  */
#define EWMA_ALPHA_30S          0.1f    /* More stable      */

/* Public API */
void processing_task_init(TX_QUEUE *processing_q, TX_QUEUE *display_q);
void processing_task_entry(ULONG arg);

#endif /* INC_PROCESSING_H_ */
