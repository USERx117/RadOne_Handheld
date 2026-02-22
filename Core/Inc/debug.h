/**
  ******************************************************************************
  * @file    debug.h
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Debug output macros
  *
  *  Usage:
  *    Set RADONE_DEBUG to 1 to enable UART debug output, 0 to disable.
  *    Include this file in any .c that needs debug output.
  *    Use DEBUG_PRINT("fmt", ...) like printf.
  *
  *  Example:
  *    DEBUG_PRINT("EVT=%d\r\n", evt);
  *    DEBUG_PRINT("active=%d next=%d\r\n", s_active, next);
  ******************************************************************************
  */

#ifndef INC_DEBUG_H_
#define INC_DEBUG_H_

#include "usart.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * Set to 1 to enable debug output, 0 to disable
 * ============================================================================ */
#define RADONE_DEBUG  1

/* ============================================================================
 * Debug macro - compiles to nothing when RADONE_DEBUG == 0
 * ============================================================================ */
#if RADONE_DEBUG
    #define DEBUG_PRINT(fmt, ...) \
        do { \
            char _dbg_buf[64]; \
            snprintf(_dbg_buf, sizeof(_dbg_buf), fmt, ##__VA_ARGS__); \
            HAL_UART_Transmit(&huart1, (uint8_t*)_dbg_buf, \
                              strlen(_dbg_buf), 100); \
        } while(0)
#else
    #define DEBUG_PRINT(fmt, ...)  do { } while(0)
#endif

#endif /* INC_DEBUG_H_ */
