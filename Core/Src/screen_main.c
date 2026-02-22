/**
  ******************************************************************************
  * @file    screen_main.c
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Main measurement screen
  ******************************************************************************
  */

#include "screen_main.h"
#include "screen_menu.h"
#include "ui_helpers.h"
#include "st7789.h"
#include "fonts.h"
#include "gfx_fonts.h"
#include "power_mode.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * Layout
 *
 *  y=0
 *  y=24  RADONE  (FreeSansBold18pt7b baseline)
 *  y=38  divider under title
 *  y=100 0.000   (FreeSansBold18pt7b baseline) - main dose
 *  y=125 uSv/h   (FreeSansBold12pt7b baseline)
 *  y=145 divider
 *  y=155 10s / 30s / CPM  (Font_8x8, 3 rows)
 *  y=195 divider
 *  y=210 divider
 *  y=235 BAT
 *  y=262 button bar
 * ============================================================================ */
#define Y_TITLE_BASE     24
#define Y_DIV_TITLE      38

#define Y_DOSE_BASE     105   /* baseline for large dose number */
#define Y_UNIT_BASE     130   /* baseline for uSv/h             */

#define Y_DIV_ROWS      148
#define Y_10S           158
#define Y_30S           172
#define Y_TICKS         186
#define Y_DIV_TICKS     200

#define Y_DIV_BAT       225
#define Y_BATTERY       235

#define COLOR_BG        ST7789_BLACK
#define COLOR_TITLE     ST7789_CYAN
#define COLOR_DOSE      ST7789_WHITE
#define COLOR_UNIT      ST7789_LIGHTGRAY
#define COLOR_LABEL     ST7789_DARKGRAY
#define COLOR_VALUE     ST7789_WHITE
#define COLOR_DIVIDER   ST7789_DARKGRAY
#define COLOR_BAT_OK    ST7789_GREEN
#define COLOR_BAT_LOW   ST7789_RED
#define COLOR_BAT_MID   ST7789_YELLOW

/* ============================================================================
 * Dirty flag cache
 * ============================================================================ */
static char s_last_dose[16]  = "";
static char s_last_10s[24]   = "";
static char s_last_30s[24]   = "";
static char s_last_ticks[24] = "";
static char s_last_bat[24]   = "";

/* ============================================================================
 * Draw helpers
 * ============================================================================ */
static void draw_divider(uint16_t y)
{
    ST7789_DrawLine(5, y, 235, y, COLOR_DIVIDER);
}

static void draw_value_right8(uint16_t x, uint16_t y, uint16_t field_w,
                               const char *str, uint16_t color)
{
    uint16_t str_w = strlen(str) * Font_8x8.width;
    uint16_t x_start = (str_w < field_w) ? (x + field_w - str_w) : x;
    ST7789_FillRect(x, y, field_w, Font_8x8.height, COLOR_BG);
    ST7789_DrawString(x_start, y, str, color, COLOR_BG, &Font_8x8);
}

static void draw_gfx_centered(uint16_t y_base, const char *str,
                               uint16_t color, const GFXfont *font)
{
    uint16_t str_w = ST7789_GFXStringWidth(str, font);
    uint16_t x = (str_w < ST7789_WIDTH) ? ((ST7789_WIDTH - str_w) / 2) : 0;
    ST7789_FillRect(0, y_base - font->yAdvance, ST7789_WIDTH,
                    font->yAdvance + 4, COLOR_BG);
    ST7789_DrawGFXString(x, y_base, str, color, COLOR_BG, font);
}

static void draw_battery(float voltage, uint8_t percent)
{
    char buf[24];
    ST7789_FillRect(0, Y_BATTERY, ST7789_WIDTH, 18, COLOR_BG);
    ST7789_DrawString(5, Y_BATTERY, "BAT", COLOR_LABEL, COLOR_BG, &Font_8x8);

    uint16_t bar_color = (percent > 50) ? COLOR_BAT_OK :
                         (percent > 20) ? COLOR_BAT_MID : COLOR_BAT_LOW;
    ST7789_DrawProgressBar(32, Y_BATTERY + 2, 120, 10, percent, bar_color, COLOR_BG);

    snprintf(buf, sizeof(buf), "%.2fV %d%%", voltage, percent);
    ST7789_DrawString(158, Y_BATTERY + 2, buf, COLOR_VALUE, COLOR_BG, &Font_8x8);
}

/* ============================================================================
 * Screen callbacks
 * ============================================================================ */
static void main_on_enter(void)
{
    /* Reset dirty flags */
    s_last_dose[0]  = '\0';
    s_last_10s[0]   = '\0';
    s_last_30s[0]   = '\0';
    s_last_ticks[0] = '\0';
    s_last_bat[0]   = '\0';

    ST7789_FillScreen(COLOR_BG);

    /* Title: RAD in cyan, ONE in white */
    uint16_t w_rad = ST7789_GFXStringWidth("RAD", &FreeSansBold18pt7b);
    uint16_t w_one = ST7789_GFXStringWidth("ONE", &FreeSansBold18pt7b);
    uint16_t tx    = (ST7789_WIDTH - w_rad - w_one) / 2;
    ST7789_DrawGFXString(tx,         Y_TITLE_BASE, "RAD",
                         COLOR_TITLE,  COLOR_BG, &FreeSansBold18pt7b);
    ST7789_DrawGFXString(tx + w_rad, Y_TITLE_BASE, "ONE",
                         ST7789_WHITE, COLOR_BG, &FreeSansBold18pt7b);
    draw_divider(Y_DIV_TITLE);

    /* Unit label - drawn once, stays static */
    uint16_t uw = ST7789_GFXStringWidth("uSv/h", &FreeSansBold12pt7b);
    ST7789_DrawGFXString((ST7789_WIDTH - uw) / 2, Y_UNIT_BASE,
                         "uSv/h", COLOR_UNIT, COLOR_BG, &FreeSansBold12pt7b);

    /* Dividers */
    draw_divider(Y_DIV_ROWS);
    draw_divider(Y_DIV_TICKS);
    draw_divider(Y_DIV_BAT);

    /* Row labels */
    ST7789_DrawString(5, Y_10S,   "10s:", COLOR_LABEL, COLOR_BG, &Font_8x8);
    ST7789_DrawString(5, Y_30S,   "30s:", COLOR_LABEL, COLOR_BG, &Font_8x8);
    ST7789_DrawString(5, Y_TICKS, "CPM:", COLOR_LABEL, COLOR_BG, &Font_8x8);

    ui_draw_button_bar("<", "Menu", ">");
}

static void main_on_data(const display_data_t *data)
{
    char buf[24];

    /* Main dose - large font, no redraw if unchanged */
    snprintf(buf, sizeof(buf), "%.3f", data->dose_rate_60s);
    if (strcmp(buf, s_last_dose) != 0) {
        strcpy(s_last_dose, buf);
        draw_gfx_centered(Y_DOSE_BASE, buf, COLOR_DOSE, &FreeSansBold18pt7b);
    }

    /* 10s */
    snprintf(buf, sizeof(buf), "%.3f uSv/h", data->dose_rate_10s);
    if (strcmp(buf, s_last_10s) != 0) {
        strcpy(s_last_10s, buf);
        draw_value_right8(37, Y_10S, 198, buf, COLOR_VALUE);
    }

    /* 30s */
    snprintf(buf, sizeof(buf), "%.3f uSv/h", data->dose_rate_30s);
    if (strcmp(buf, s_last_30s) != 0) {
        strcpy(s_last_30s, buf);
        draw_value_right8(37, Y_30S, 198, buf, COLOR_VALUE);
    }

    /* CPM */
    snprintf(buf, sizeof(buf), "%lu ticks", (unsigned long)data->tick_count);
    if (strcmp(buf, s_last_ticks) != 0) {
        strcpy(s_last_ticks, buf);
        draw_value_right8(37, Y_TICKS, 198, buf, COLOR_VALUE);
    }

    /* Battery */
    snprintf(buf, sizeof(buf), "%.2fV %d%%",
             data->battery_voltage, data->battery_percent);
    if (strcmp(buf, s_last_bat) != 0) {
        strcpy(s_last_bat, buf);
        draw_battery(data->battery_voltage, data->battery_percent);
    }
}

static screen_id_t main_on_event(btn_event_t evt)
{
    switch (evt) {
        case BTN_MID:
            return SCREEN_MENU;
        case BTN_MID_LONG:
            if (power_mode_get() == POWER_MODE_BUZZER_ONLY)
                power_mode_set(POWER_MODE_DEFAULT);
            return SCREEN_MAIN;
        default:
            return SCREEN_MAIN;
    }
}

const screen_t screen_main = {
    .on_enter = main_on_enter,
    .on_data  = main_on_data,
    .on_event = main_on_event,
};
