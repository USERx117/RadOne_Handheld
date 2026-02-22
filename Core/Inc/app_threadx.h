/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.h
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - shared types and queue declarations
  *
  *          Single include point for all inter-task communication types.
  *          All modules include this header.
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_THREADX_H
#define __APP_THREADX_H
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "tx_api.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/**
 * @brief ISR -> Sensor Task  (via g_sensor_queue)
 * Size: 8 bytes = 2x ULONG
 */
typedef struct {
    uint32_t timestamp;     /* HAL_GetTick() at moment of detection */
    uint32_t padding;       /* Reserved, always 0                   */
} sensor_event_t;

/**
 * @brief Processing Task -> Display Task  (via g_display_queue)
 * Size: 28 bytes = 7x ULONG
 *
 * Complete snapshot sent after each 1-second calculation.
 * Display task renders from this alone - no shared globals needed.
 */
typedef struct {
    uint32_t tick_count;        /* Lifetime tick counter            */
    float    dose_rate_10s;     /* uSv/h  (EWMA, 10s window)       */
    float    dose_rate_30s;     /* uSv/h  (EWMA, 30s window)       */
    float    dose_rate_60s;     /* uSv/h  (true CPM, 60s window)   */
    uint32_t uptime_s;          /* System uptime in seconds         */
    float    battery_voltage;   /* Battery voltage in V             */
    uint8_t  battery_percent;   /* Battery level 0-100%             */
    uint8_t  pad[3];            /* Explicit padding to 28 bytes     */
} display_data_t;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* Sensor Queue: ISR -> Sensor Task */
#define SENSOR_QUEUE_DEPTH      10
#define SENSOR_EVENT_MSG_WORDS  2       /* 8  bytes / sizeof(ULONG) = 2 */

/* Display Queue: Processing Task -> Display Task */
#define DISPLAY_QUEUE_DEPTH     3
#define DISPLAY_DATA_MSG_WORDS  7       /* 28 bytes / sizeof(ULONG) = 7 */

/* USER CODE END EC */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Main thread defines -------------------------------------------------------*/
/* USER CODE BEGIN MTD */
/* USER CODE END MTD */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
UINT App_ThreadX_Init(VOID *memory_ptr);
void MX_ThreadX_Init(void);
void valueNotSetted(ULONG thread_input);

/* USER CODE BEGIN EFP */
/* USER CODE END EFP */

/* USER CODE BEGIN 1 */
/* Inter-task queues - defined in app_threadx.c */
extern TX_QUEUE g_sensor_queue;
extern TX_QUEUE g_processing_queue;
extern TX_QUEUE g_display_queue;
/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /* __APP_THREADX_H */
