/**
  ******************************************************************************
  * @file    screen_menu.c
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Settings menu screen
  ******************************************************************************
  */

#include "screen_menu.h"
#include "screen_main.h"
#include "ui_helpers.h"
#include "st7789.h"
#include "fonts.h"
#include "gfx_fonts.h"
#include "tim.h"
#include <string.h>

/* ============================================================================
 * Menu items
 * ============================================================================ */
typedef enum {
    MENU_BACK = 0,
    MENU_BRIGHTNESS,
    MENU_PERFORMANCE,
    MENU_BATSAVE,
    MENU_GRAPH,
    MENU_COUNT
} menu_item_t;

static const char *s_labels[MENU_COUNT] = {
    "Back",
    "Brightness",
    "Performance",
    "Bat.Save",
    "Graph",
};

/* ============================================================================
 * Brightness - TIM2 ARR=999, 10 steps
 * ============================================================================ */
#define BRIGHTNESS_MIN      100
#define BRIGHTNESS_MAX      999
#define BRIGHTNESS_STEP     100
#define BRIGHTNESS_DEFAULT  800

static uint16_t s_brightness = BRIGHTNESS_DEFAULT;

/* Forward declarations */
static void menu_draw_brightness_bar(void);
static void menu_draw_items(void);
static void menu_draw_detail(void);

static void brightness_apply(void)
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, s_brightness);
}

static void brightness_inc(void)
{
    if (s_brightness + BRIGHTNESS_STEP <= BRIGHTNESS_MAX)
        s_brightness += BRIGHTNESS_STEP;
    else
        s_brightness = BRIGHTNESS_MAX;
    brightness_apply();
    menu_draw_brightness_bar();
}

static void brightness_dec(void)
{
    if (s_brightness - BRIGHTNESS_STEP >= BRIGHTNESS_MIN)
        s_brightness -= BRIGHTNESS_STEP;
    else
        s_brightness = BRIGHTNESS_MIN;
    brightness_apply();
    menu_draw_brightness_bar();
}

/* ============================================================================
 * Layout
 * ============================================================================ */
#define COLOR_BG        ST7789_BLACK
#define COLOR_ITEM      ST7789_DARKGRAY
#define COLOR_SELECTED  ST7789_WHITE
#define COLOR_INDICATOR ST7789_CYAN
#define COLOR_DIVIDER   ST7789_DARKGRAY

#define Y_TITLE         20
#define Y_DIVIDER_T     38
#define Y_ITEMS         86
#define Y_DIVIDER_B     108
#define Y_DETAIL        120

static menu_item_t s_selected = MENU_BACK;

static void menu_draw_brightness_bar(void)
{
    uint8_t pct = (uint8_t)((s_brightness * 100UL) / BRIGHTNESS_MAX);
    ST7789_FillRect(20, Y_DETAIL + 14, 200, 14, COLOR_BG);
    ST7789_DrawProgressBar(20, Y_DETAIL + 14, 200, 14, pct,
                           ST7789_CYAN, COLOR_BG);
}

static void menu_draw_items(void)
{
    ST7789_FillRect(0, 45, ST7789_WIDTH, 60, COLOR_BG);

    /* Previous item (left, dimmed) */
    menu_item_t prev = (s_selected + MENU_COUNT - 1) % MENU_COUNT;
    menu_item_t next = (s_selected + 1) % MENU_COUNT;

    ST7789_DrawString(8,   Y_ITEMS, s_labels[prev],      COLOR_ITEM,     COLOR_BG, &Font_8x8);
    ST7789_DrawString(88,  Y_ITEMS, s_labels[s_selected], COLOR_SELECTED, COLOR_BG, &Font_8x8);
    ST7789_DrawString(170, Y_ITEMS, s_labels[next],       COLOR_ITEM,     COLOR_BG, &Font_8x8);

    /* Underline selected */
    uint16_t sel_w = strlen(s_labels[s_selected]) * Font_8x8.width;
    ST7789_FillRect(88, Y_ITEMS + 10, 80, 2, COLOR_BG);
    ST7789_FillRect(88, Y_ITEMS + 10, sel_w, 2, COLOR_INDICATOR);
}

static void menu_draw_detail(void)
{
    ST7789_FillRect(0, Y_DETAIL, ST7789_WIDTH, 130, COLOR_BG);

    switch (s_selected) {
        case MENU_BRIGHTNESS:
            ST7789_DrawString(20, Y_DETAIL,
                              "Brightness", ST7789_LIGHTGRAY, COLOR_BG, &Font_8x8);
            menu_draw_brightness_bar();
            ST7789_DrawString(20, Y_DETAIL + 34,
                              "Hold < > to adjust", ST7789_DARKGRAY, COLOR_BG, &Font_8x8);
            break;
        case MENU_PERFORMANCE:
            ST7789_DrawString(20, Y_DETAIL,
                              "Performance Mode", ST7789_LIGHTGRAY, COLOR_BG, &Font_8x8);
            ST7789_DrawString(20, Y_DETAIL + 16,
                              "Faster update rate", ST7789_DARKGRAY, COLOR_BG, &Font_8x8);
            ST7789_DrawString(20, Y_DETAIL + 30,
                              "Press MID to toggle", ST7789_DARKGRAY, COLOR_BG, &Font_8x8);
            break;
        case MENU_BATSAVE:
            ST7789_DrawString(20, Y_DETAIL,
                              "Battery Save Mode", ST7789_LIGHTGRAY, COLOR_BG, &Font_8x8);
            ST7789_DrawString(20, Y_DETAIL + 16,
                              "Reduced brightness", ST7789_DARKGRAY, COLOR_BG, &Font_8x8);
            ST7789_DrawString(20, Y_DETAIL + 30,
                              "Press MID to toggle", ST7789_DARKGRAY, COLOR_BG, &Font_8x8);
            break;
        case MENU_GRAPH:
            ST7789_DrawString(20, Y_DETAIL,
                              "5min dose graph", ST7789_LIGHTGRAY, COLOR_BG, &Font_8x8);
            ST7789_DrawString(20, Y_DETAIL + 16,
                              "[Coming soon]", ST7789_DARKGRAY, COLOR_BG, &Font_8x8);
            break;
        case MENU_BACK:
            ST7789_DrawString(20, Y_DETAIL,
                              "Return to main screen", ST7789_LIGHTGRAY, COLOR_BG, &Font_8x8);
            break;
        default:
            break;
    }
}

static void menu_on_enter(void)
{
    s_selected = MENU_BACK;
    ST7789_FillScreen(COLOR_BG);

    /* Title */
    uint16_t tw = ST7789_GFXStringWidth("Settings", &FreeSansBold12pt7b);
    ST7789_DrawGFXString((ST7789_WIDTH - tw) / 2, Y_TITLE,
                         "Settings", ST7789_CYAN, COLOR_BG, &FreeSansBold12pt7b);

    ST7789_DrawLine(5, Y_DIVIDER_T, 235, Y_DIVIDER_T, COLOR_DIVIDER);
    ST7789_DrawLine(5, Y_DIVIDER_B, 235, Y_DIVIDER_B, COLOR_DIVIDER);

    menu_draw_items();
    menu_draw_detail();

    /* Button bar: < | Select | > */
    ui_draw_button_bar("<", "Select", ">");
}

static void menu_on_data(const display_data_t *data)
{
    (void)data;
}

static screen_id_t menu_on_event(btn_event_t evt)
{
    switch (evt) {
        case BTN_LEFT:
            s_selected = (s_selected + MENU_COUNT - 1) % MENU_COUNT;
            menu_draw_items();
            menu_draw_detail();
            /* Update button bar context for brightness */
            if (s_selected == MENU_BRIGHTNESS)
                ui_draw_button_bar("< Dim", "Select", "Bright >");
            else
                ui_draw_button_bar("<", "Select", ">");
            break;

        case BTN_RIGHT:
            s_selected = (s_selected + 1) % MENU_COUNT;
            menu_draw_items();
            menu_draw_detail();
            if (s_selected == MENU_BRIGHTNESS)
                ui_draw_button_bar("< Dim", "Select", "Bright >");
            else
                ui_draw_button_bar("<", "Select", ">");
            break;

        case BTN_MID:
            if (s_selected == MENU_BACK)  return SCREEN_MAIN;
            if (s_selected == MENU_GRAPH) return SCREEN_MAIN; /* placeholder */
            break;

        case BTN_LEFT_LONG:
            if (s_selected == MENU_BRIGHTNESS) brightness_dec();
            break;

        case BTN_RIGHT_LONG:
            if (s_selected == MENU_BRIGHTNESS) brightness_inc();
            break;

        default:
            break;
    }
    return SCREEN_MENU;
}

const screen_t screen_menu = {
    .on_enter = menu_on_enter,
    .on_data  = menu_on_data,
    .on_event = menu_on_event,
};
