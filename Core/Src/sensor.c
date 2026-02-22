/**
 ******************************************************************************
 * @file    sensor.c
 * @author  Florian Gotschim
 * @brief   RadOne Handheld - Sensor Task
 ******************************************************************************
 */

#include "sensor.h"
#include "peripherals.h"
#include "main.h"

uint32_t g_tick_buffer[TICK_BUFFER_SIZE] = { 0 };
uint32_t g_tick_buffer_index = 0;
uint32_t g_tick_count = 0;

static TX_QUEUE *s_sensor_queue = NULL;
static TX_QUEUE *s_processing_queue = NULL;
static TX_SEMAPHORE s_second_sem;

void sensor_task_init(TX_QUEUE *sensor_q, TX_QUEUE *processing_q) {
	s_sensor_queue = sensor_q;
	s_processing_queue = processing_q;
	tx_semaphore_create(&s_second_sem, "SecondSem", 0);
}

void sensor_isr_handler(void) {
	if (s_sensor_queue == NULL)
		return;

	sensor_event_t event;
	event.timestamp = HAL_GetTick();
	event.padding = 0;
	tx_queue_send(s_sensor_queue, &event, TX_NO_WAIT);
}

void sensor_second_elapsed(void) {
	tx_semaphore_put(&s_second_sem);
}

void sensor_task_entry(ULONG arg) {
	(void) arg;
	sensor_event_t event;

	while (1) {
		if (tx_queue_receive(s_sensor_queue, &event, 10) == TX_SUCCESS) {
			g_tick_count++;
			g_tick_buffer[g_tick_buffer_index]++;
			peripherals_on_tick();
		}

		if (tx_semaphore_get(&s_second_sem, TX_NO_WAIT) == TX_SUCCESS) {
			g_tick_buffer_index = (g_tick_buffer_index + 1) % TICK_BUFFER_SIZE;
			g_tick_buffer[g_tick_buffer_index] = 0;

			sensor_data_t data;
			data.tick_count = g_tick_count;
			data.uptime_s = HAL_GetTick() / 100;
			tx_queue_send(s_processing_queue, &data, TX_NO_WAIT);
		}
	}
}
