/**
  ******************************************************************************
  * @file    ui_helpers.h
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Shared UI helper functions
  *
  *          Reusable UI elements used across multiple screens.
  *          Keeps screen_*.c files clean and consistent.
  ******************************************************************************
  */

#ifndef INC_UI_HELPERS_H_
#define INC_UI_HELPERS_H_

#include "st7789.h"
#include <stdint.h>

/* ============================================================================
 * Button bar - shown at bottom of every screen
 *
 * Draws three labeled buttons with rounded outline:
 *   [left_label]   [mid_label]   [right_label]
 *
 * Use "<" ">" "^" "v" for navigation arrows
 * Use "Menu" "Select" "Back" for context labels
 * Pass NULL or "" to draw an empty/disabled button
 * ============================================================================ */
void ui_draw_button_bar(const char *left, const char *mid, const char *right);

/* ============================================================================
 * Rounded rectangle outline
 * r = corner radius in pixels
 * ============================================================================ */
void ui_draw_rounded_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                          uint8_t r, uint16_t color);

/* ============================================================================
 * Centered string helper (Font_8x8)
 * Draws string centered within a field of width field_w starting at x
 * ============================================================================ */
void ui_draw_centered_str(uint16_t x, uint16_t y, uint16_t field_w,
                          const char *str, uint16_t color, uint16_t bg);

#endif /* INC_UI_HELPERS_H_ */
