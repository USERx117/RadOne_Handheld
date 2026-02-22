/**
  ******************************************************************************
  * @file    display.c
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Display Task (Screen Manager wrapper)
  ******************************************************************************
  */

#include "display.h"
#include "st7789.h"
#include "screen_manager.h"
#include "peripherals.h"
#include "usart.h"
#include "stdio.h"

static TX_QUEUE *s_display_queue = NULL;
static TX_QUEUE *s_button_queue  = NULL;

void display_task_init(TX_QUEUE *display_q, TX_QUEUE *button_q)
{
    s_display_queue = display_q;
    s_button_queue  = button_q;
}

void display_task_entry(ULONG arg)
{
    (void)arg;

    tx_thread_sleep(50);
    ST7789_Init();
    screen_manager_init();

    while (1)
    {
        display_data_t data;
        if (tx_queue_receive(s_display_queue, &data, 10) == TX_SUCCESS)
            screen_manager_data(&data);

    }
}
