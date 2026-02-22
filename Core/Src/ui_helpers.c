/**
  ******************************************************************************
  * @file    ui_helpers.c
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Shared UI helper functions
  ******************************************************************************
  */

#include "ui_helpers.h"
#include "fonts.h"
#include <string.h>

/* ============================================================================
 * Button bar layout
 * Three buttons at bottom of screen, each with rounded outline
 *
 *  x=5        x=85       x=155
 *  ┌──────┐   ┌──────┐   ┌──────┐
 *  │  <   │   │ Menu │   │  >   │
 *  └──────┘   └──────┘   └──────┘
 *  y=262                   y=275
 * ============================================================================ */
#define BTN_BAR_Y       262
#define BTN_BAR_H        14
#define BTN_BAR_RADIUS    3
#define BTN_L_X           5
#define BTN_M_X          85
#define BTN_R_X         160
#define BTN_W_NARROW     70   /* Left and Right buttons */
#define BTN_W_WIDE       65   /* Mid button */
#define BTN_COLOR_OUTLINE ST7789_DARKGRAY
#define BTN_COLOR_TEXT    ST7789_WHITE
#define BTN_COLOR_BG      ST7789_BLACK

/* ============================================================================
 * Rounded rectangle helper
 * Draws outline only (no fill) with radius r corners
 * ============================================================================ */
void ui_draw_rounded_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                          uint8_t r, uint16_t color)
{
    /* Straight edges */
    ST7789_DrawLine(x + r,     y,         x + w - r, y,         color); /* top    */
    ST7789_DrawLine(x + r,     y + h,     x + w - r, y + h,     color); /* bottom */
    ST7789_DrawLine(x,         y + r,     x,         y + h - r, color); /* left   */
    ST7789_DrawLine(x + w,     y + r,     x + w,     y + h - r, color); /* right  */

    /* Corners - approximated with 3 pixels each */
    /* Top-left */
    ST7789_DrawPixel(x + 1, y + 1, color);
    ST7789_DrawPixel(x + 1, y + 2, color);
    ST7789_DrawPixel(x + 2, y + 1, color);
    /* Top-right */
    ST7789_DrawPixel(x + w - 1, y + 1, color);
    ST7789_DrawPixel(x + w - 1, y + 2, color);
    ST7789_DrawPixel(x + w - 2, y + 1, color);
    /* Bottom-left */
    ST7789_DrawPixel(x + 1, y + h - 1, color);
    ST7789_DrawPixel(x + 1, y + h - 2, color);
    ST7789_DrawPixel(x + 2, y + h - 1, color);
    /* Bottom-right */
    ST7789_DrawPixel(x + w - 1, y + h - 1, color);
    ST7789_DrawPixel(x + w - 1, y + h - 2, color);
    ST7789_DrawPixel(x + w - 2, y + h - 1, color);
}

/* ============================================================================
 * Center a string within a field
 * ============================================================================ */
void ui_draw_centered_str(uint16_t x, uint16_t y, uint16_t field_w,
                          const char *str, uint16_t color, uint16_t bg)
{
    if (!str || str[0] == '\0') return;
    uint16_t str_w = strlen(str) * Font_8x8.width;
    uint16_t x_start = (str_w < field_w) ? (x + (field_w - str_w) / 2) : x;
    ST7789_DrawString(x_start, y, str, color, bg, &Font_8x8);
}

/* ============================================================================
 * Button bar
 * ============================================================================ */
void ui_draw_button_bar(const char *left, const char *mid, const char *right)
{
    /* Clear entire bar area */
    ST7789_FillRect(0, BTN_BAR_Y - 2, ST7789_WIDTH, BTN_BAR_H + 6, BTN_COLOR_BG);

    uint16_t text_y = BTN_BAR_Y + (BTN_BAR_H - Font_8x8.height) / 2;

    /* Left button */
    if (left && left[0] != '\0') {
        ui_draw_rounded_rect(BTN_L_X, BTN_BAR_Y, BTN_W_NARROW, BTN_BAR_H,
                             BTN_BAR_RADIUS, BTN_COLOR_OUTLINE);
        ui_draw_centered_str(BTN_L_X + 2, text_y, BTN_W_NARROW - 4,
                             left, BTN_COLOR_TEXT, BTN_COLOR_BG);
    }

    /* Mid button */
    if (mid && mid[0] != '\0') {
        ui_draw_rounded_rect(BTN_M_X, BTN_BAR_Y, BTN_W_WIDE, BTN_BAR_H,
                             BTN_BAR_RADIUS, BTN_COLOR_OUTLINE);
        ui_draw_centered_str(BTN_M_X + 2, text_y, BTN_W_WIDE - 4,
                             mid, BTN_COLOR_TEXT, BTN_COLOR_BG);
    }

    /* Right button */
    if (right && right[0] != '\0') {
        ui_draw_rounded_rect(BTN_R_X, BTN_BAR_Y, BTN_W_NARROW, BTN_BAR_H,
                             BTN_BAR_RADIUS, BTN_COLOR_OUTLINE);
        ui_draw_centered_str(BTN_R_X + 2, text_y, BTN_W_NARROW - 4,
                             right, BTN_COLOR_TEXT, BTN_COLOR_BG);
    }
}
