/**
  ******************************************************************************
  * @file    processing.c
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Processing Task
  *
  *  Power mode behaviour:
  *    DEFAULT:     Send display update every second (driven by sensor tick)
  *    PERF:        Send display update every 500ms (driven by sensor tick)
  *    BATSAVE:     Send display update every 5s only
  *    BUZZER_ONLY: Sleep, no calculations, no display updates
  ******************************************************************************
  */

#include "processing.h"
#include "sensor.h"
#include "peripherals.h"
#include "power_mode.h"
#include "debug.h"
#include <stdio.h>
#include <string.h>

static TX_QUEUE *s_processing_queue = NULL;
static TX_QUEUE *s_display_queue    = NULL;

static float    s_ewma_10s       = 0.0f;
static float    s_ewma_30s       = 0.0f;
static uint32_t s_last_display_s = 0;

void processing_task_init(TX_QUEUE *processing_q, TX_QUEUE *display_q)
{
    s_processing_queue = processing_q;
    s_display_queue    = display_q;
}

static uint32_t sum_window(uint32_t window_size)
{
    uint32_t total = 0;
    uint32_t idx   = g_tick_buffer_index;

    for (uint32_t i = 0; i < window_size && i < TICK_BUFFER_SIZE; i++)
    {
        total += g_tick_buffer[idx];
        idx = (idx == 0) ? TICK_BUFFER_SIZE - 1 : idx - 1;
    }
    return total;
}

static uint32_t refresh_interval_s(void)
{
    switch (power_mode_get()) {
        case POWER_MODE_PERF:    return 0;
        case POWER_MODE_BATSAVE: return 5;
        default:                 return 1;
    }
}

void processing_task_entry(ULONG arg)
{
    (void)arg;
    sensor_data_t  incoming;
    display_data_t snapshot;

    while (1)
    {
        /* BUZZER_ONLY: sleep, nothing to process */
        if (!power_is_processing_active()) {
            tx_thread_sleep(100);
            continue;
        }

        if (tx_queue_receive(s_processing_queue, &incoming, TX_WAIT_FOREVER) != TX_SUCCESS)
            continue;

        if (!power_is_processing_active())
            continue;

        /* Calculate dose rates */
        uint32_t ticks_10s = sum_window(10);
        uint32_t ticks_30s = sum_window(30);
        uint32_t ticks_60s = sum_window(60);

        float cpm_10s = (ticks_10s / 10.0f) * 60.0f;
        float cpm_30s = (ticks_30s / 30.0f) * 60.0f;
        float cpm_60s = (ticks_60s / 60.0f) * 60.0f;

        if (s_ewma_10s == 0.0f && cpm_10s > 0.0f) s_ewma_10s = cpm_10s;
        if (s_ewma_30s == 0.0f && cpm_30s > 0.0f) s_ewma_30s = cpm_30s;

        s_ewma_10s = EWMA_ALPHA_10S * cpm_10s + (1.0f - EWMA_ALPHA_10S) * s_ewma_10s;
        s_ewma_30s = EWMA_ALPHA_30S * cpm_30s + (1.0f - EWMA_ALPHA_30S) * s_ewma_30s;

        /* Build snapshot */
        snapshot.tick_count      = incoming.tick_count;
        snapshot.dose_rate_10s   = s_ewma_10s * AL54_CPM_TO_USV_H;
        snapshot.dose_rate_30s   = s_ewma_30s * AL54_CPM_TO_USV_H;
        snapshot.dose_rate_60s   = cpm_60s    * AL54_CPM_TO_USV_H;
        snapshot.uptime_s        = incoming.uptime_s;
        snapshot.battery_voltage = battery_get_voltage();
        snapshot.battery_percent = battery_get_percent();
        snapshot.pad[0] = snapshot.pad[1] = snapshot.pad[2] = 0;

        /* Update LED colour - always, not just on display refresh */
        peripherals_update_dose(snapshot.dose_rate_60s);

        /* Check if enough time has passed for a display update */
        uint32_t interval = refresh_interval_s();
        if (incoming.uptime_s - s_last_display_s < interval)
            continue;

        s_last_display_s = incoming.uptime_s;

        DEBUG_PRINT("PROC uptime=%lu interval=%lu dose=%.4f\r\n",
                    incoming.uptime_s, interval,
                    (double)snapshot.dose_rate_60s);

        tx_queue_flush(&g_display_queue);
        tx_queue_send(s_display_queue, &snapshot, TX_NO_WAIT);
    }
}
