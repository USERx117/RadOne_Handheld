/**
  ******************************************************************************
  * @file    sensor.h
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Sensor Task
  *
  *          Responsibilities:
  *          - Receive sensor_event_t from g_sensor_queue (posted by ISR)
  *          - Maintain lifetime tick counter
  *          - Maintain 60-slot circular tick buffer (1 slot = 1 second)
  *          - Forward raw tick data to Processing Task via g_processing_queue
  ******************************************************************************
  */

#ifndef INC_SENSOR_H_
#define INC_SENSOR_H_

#include "app_threadx.h"

/* Task configuration */
#define SENSOR_TASK_PRIORITY    5
#define SENSOR_TASK_STACK_SIZE  2048

/* Circular tick buffer size (seconds of history) */
#define TICK_BUFFER_SIZE        60

/**
 * @brief Raw tick data: Sensor Task -> Processing Task
 * Size: 8 bytes = 2x ULONG
 */
typedef struct {
    uint32_t tick_count;        /* Lifetime tick count (updated each tick)  */
    uint32_t uptime_s;          /* Uptime in seconds (updated each second)  */
} sensor_data_t;

#define SENSOR_DATA_MSG_WORDS   2       /* 8 bytes / sizeof(ULONG) = 2 */
#define SENSOR_DATA_QUEUE_DEPTH 5

/* Public API */
void sensor_task_init(TX_QUEUE *sensor_q, TX_QUEUE *processing_q);
void sensor_task_entry(ULONG arg);

/* Called from HAL_GPIO_EXTI_Callback in app_threadx.c - ultra-fast ISR handler */
void sensor_isr_handler(void);

/* Called from 1s measurement_timer_callback in app_threadx.c */
void sensor_second_elapsed(void);

/* Read-only access to tick buffer for processing task */
extern uint32_t g_tick_buffer[TICK_BUFFER_SIZE];
extern uint32_t g_tick_buffer_index;
extern uint32_t g_tick_count;

#endif /* INC_SENSOR_H_ */
