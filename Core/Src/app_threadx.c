/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    app_threadx.c
 * @author  Florian Gotschim
 * @brief   RadOne Handheld - ThreadX orchestrator
 *
 *          Creates all queues and tasks, routes HAL callbacks.
 *          All logic lives in sensor.c / processing.c / display.c / peripherals.c
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_threadx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "sensor.h"
#include "processing.h"
#include "display.h"
#include "peripherals.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
TX_QUEUE g_sensor_queue;
TX_QUEUE g_processing_queue;
TX_QUEUE g_display_queue;
TX_QUEUE g_button_queue;

static ULONG s_sensor_q_storage[SENSOR_QUEUE_DEPTH * SENSOR_EVENT_MSG_WORDS];
static ULONG s_processing_q_storage[SENSOR_DATA_QUEUE_DEPTH
		* SENSOR_DATA_MSG_WORDS];
static ULONG s_display_q_storage[DISPLAY_QUEUE_DEPTH * DISPLAY_DATA_MSG_WORDS];
static ULONG s_button_q_storage[8]; /* 8 x btn_event_t */

static TX_THREAD s_sensor_task;
static TX_THREAD s_processing_task;
static TX_THREAD s_display_task;
static TX_THREAD s_peripherals_task;

static ULONG s_sensor_stack[SENSOR_TASK_STACK_SIZE / sizeof(ULONG)];
static ULONG s_processing_stack[PROCESSING_TASK_STACK_SIZE / sizeof(ULONG)];
static ULONG s_display_stack[DISPLAY_TASK_STACK_SIZE / sizeof(ULONG)];
static ULONG s_peripherals_stack[PERIPHERALS_TASK_STACK_SIZE / sizeof(ULONG)];

static TX_TIMER s_measurement_timer;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void measurement_timer_cb(ULONG arg);
/* USER CODE END PFP */

/**
  * @brief  Application ThreadX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_ThreadX_Init(VOID *memory_ptr)
{
  UINT ret = TX_SUCCESS;

  /* USER CODE BEGIN App_ThreadX_MEM_POOL */
	(void) memory_ptr;
  /* USER CODE END App_ThreadX_MEM_POOL */

  /* USER CODE BEGIN App_ThreadX_Init */
	ret = tx_queue_create(&g_sensor_queue, "SensorQ", SENSOR_EVENT_MSG_WORDS,
			s_sensor_q_storage, sizeof(s_sensor_q_storage));
	if (ret != TX_SUCCESS) {
		Error_Handler();
	}

	ret = tx_queue_create(&g_processing_queue, "ProcessingQ",
			SENSOR_DATA_MSG_WORDS, s_processing_q_storage,
			sizeof(s_processing_q_storage));
	if (ret != TX_SUCCESS) {
		Error_Handler();
	}

	ret = tx_queue_create(&g_display_queue, "DisplayQ", DISPLAY_DATA_MSG_WORDS,
			s_display_q_storage, sizeof(s_display_q_storage));
	if (ret != TX_SUCCESS) {
		Error_Handler();
	}

	ret = tx_queue_create(&g_button_queue, "ButtonQ", 1, /* 1 ULONG per message */
	s_button_q_storage, sizeof(s_button_q_storage));
	if (ret != TX_SUCCESS) {
		Error_Handler();
	}

	sensor_task_init(&g_sensor_queue, &g_processing_queue);
	processing_task_init(&g_processing_queue, &g_display_queue);
	display_task_init(&g_display_queue, &g_button_queue);
	peripherals_task_init();

	ret = tx_thread_create(&s_sensor_task, "SensorTask", sensor_task_entry, 0,
			s_sensor_stack, sizeof(s_sensor_stack), SENSOR_TASK_PRIORITY,
			SENSOR_TASK_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);
	if (ret != TX_SUCCESS) {
		Error_Handler();
	}

	ret = tx_thread_create(&s_processing_task, "ProcessingTask",
			processing_task_entry, 0, s_processing_stack,
			sizeof(s_processing_stack), PROCESSING_TASK_PRIORITY,
			PROCESSING_TASK_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);
	if (ret != TX_SUCCESS) {
		Error_Handler();
	}

	ret = tx_thread_create(&s_display_task, "DisplayTask", display_task_entry,
			0, s_display_stack, sizeof(s_display_stack), DISPLAY_TASK_PRIORITY,
			DISPLAY_TASK_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);
	if (ret != TX_SUCCESS) {
		Error_Handler();
	}

	ret = tx_thread_create(&s_peripherals_task, "PeripheralsTask",
			peripherals_task_entry, 0, s_peripherals_stack,
			sizeof(s_peripherals_stack), PERIPHERALS_TASK_PRIORITY,
			PERIPHERALS_TASK_PRIORITY, TX_NO_TIME_SLICE, TX_AUTO_START);
	if (ret != TX_SUCCESS) {
		Error_Handler();
	}

	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 800);

	ret = tx_timer_create(&s_measurement_timer, "MeasTimer",
			measurement_timer_cb, 0, 1000, 1000, TX_AUTO_ACTIVATE);
	if (ret != TX_SUCCESS) {
		Error_Handler();
	}

  /* USER CODE END App_ThreadX_Init */

  return ret;
}

  /**
  * @brief  Function that implements the kernel's initialization.
  * @param  None
  * @retval None
  */
void MX_ThreadX_Init(void)
{
  /* USER CODE BEGIN Before_Kernel_Start */
  /* USER CODE END Before_Kernel_Start */

  tx_kernel_enter();

  /* USER CODE BEGIN Kernel_Start_Error */
  /* USER CODE END Kernel_Start_Error */
}

/* USER CODE BEGIN 1 */
static void measurement_timer_cb(ULONG arg) {
	(void) arg;
	sensor_second_elapsed();
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == AL54_DIGITAL_IN_Pin) {
		sensor_isr_handler();
	}
}
/* USER CODE END 1 */
