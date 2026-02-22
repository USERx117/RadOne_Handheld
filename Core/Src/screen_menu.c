/**
  ******************************************************************************
  * @file    screen_menu.c
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Settings menu screen
  *
  *  Horizontal main menu: Back | Bright | Power | Graph
  *
  *  Bright (after MID):
  *    - Brightness slider active, LEFT/RIGHT adjust, MID = back
  *
  *  Power (after MID):
  *    - Vertical submenu on same screen
  *    - LEFT = up, RIGHT = down, MID = toggle/select
  *    - "Back" in submenu returns to horizontal nav
  ******************************************************************************
  */

#include "screen_menu.h"
#include "screen_main.h"
#include "ui_helpers.h"
#include "st7789.h"
#include "fonts.h"
#include "gfx_fonts.h"
#include "tim.h"
#include "debug.h"
#include "power_mode.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * Focus state - what is currently "active"
 * ============================================================================ */
typedef enum {
    FOCUS_HORIZONTAL = 0,  /* navigating main menu left/right */
    FOCUS_BRIGHT,          /* brightness slider active         */
    FOCUS_POWER,           /* power vertical submenu active    */
} focus_t;

/* ============================================================================
 * Main menu items
 * ============================================================================ */
typedef enum {
    MAIN_BACK = 0,
    MAIN_BRIGHT,
    MAIN_POWER,
    MAIN_GRAPH,
    MAIN_COUNT
} main_item_t;

static const char *s_main_labels[MAIN_COUNT] = {
    "Back",
    "Bright",
    "Power",
    "Graph",
};

/* ============================================================================
 * Power submenu items
 * ============================================================================ */
typedef enum {
    POWER_BACK = 0,
    POWER_PERF,
    POWER_BATSAVE,
    POWER_BUZZER,
    POWER_COUNT
} power_item_t;

static const char *s_power_labels[POWER_COUNT] = {
    "Back",
    "Perf.Mode",
    "Bat.Save",
    "Buzz.Only",
};

/* ============================================================================
 * State
 * ============================================================================ */
static focus_t      s_focus     = FOCUS_HORIZONTAL;
static main_item_t  s_main_sel  = MAIN_BACK;
static power_item_t s_power_sel = POWER_BACK;

/* Power mode state managed via power_mode.h */

/* ============================================================================
 * Brightness
 * ============================================================================ */
#define BRIGHTNESS_MIN      100
#define BRIGHTNESS_MAX      999
#define BRIGHTNESS_STEP     100
#define BRIGHTNESS_DEFAULT  800
#define BRIGHTNESS_BATSAVE  300

static uint16_t s_brightness       = BRIGHTNESS_DEFAULT;
static uint16_t s_brightness_saved = BRIGHTNESS_DEFAULT;  /* saved before BatSave */

static void brightness_apply(void)
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, s_brightness);
}

/* Read actual brightness from hardware register - always correct
 * regardless of who set it (BatSave, manual, etc.) */
static uint16_t brightness_display(void)
{
    return (uint16_t)__HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_2);
}

static void brightness_change(int8_t dir)
{
    if (dir > 0) {
        if (s_brightness + BRIGHTNESS_STEP <= BRIGHTNESS_MAX)
            s_brightness += BRIGHTNESS_STEP;
        else
            s_brightness = BRIGHTNESS_MAX;
    } else {
        if (s_brightness >= BRIGHTNESS_MIN + BRIGHTNESS_STEP)
            s_brightness -= BRIGHTNESS_STEP;
        else
            s_brightness = BRIGHTNESS_MIN;
    }
    brightness_apply();
}

/* ============================================================================
 * Layout constants
 * ============================================================================ */
#define COLOR_BG        ST7789_BLACK
#define COLOR_ITEM      ST7789_DARKGRAY
#define COLOR_SELECTED  ST7789_WHITE
#define COLOR_ACTIVE    ST7789_CYAN
#define COLOR_INDICATOR ST7789_CYAN
#define COLOR_DIVIDER   ST7789_DARKGRAY
#define COLOR_ON        ST7789_GREEN
#define COLOR_OFF       ST7789_DARKGRAY

#define Y_TITLE         20
#define Y_DIVIDER_T     38
#define Y_ITEMS         86
#define Y_DIVIDER_B     108
#define Y_DETAIL        122

/* ============================================================================
 * Forward declarations
 * ============================================================================ */
static void draw_main_items(void);
static void draw_detail(void);
static void draw_brightness_detail(void);
static void draw_power_detail(void);
static void update_button_bar(void);

/* ============================================================================
 * Draw: horizontal menu items
 * ============================================================================ */
static void draw_main_items(void)
{
    ST7789_FillRect(0, 45, ST7789_WIDTH, 65, COLOR_BG);

    uint8_t prev = (s_main_sel + MAIN_COUNT - 1) % MAIN_COUNT;
    uint8_t next = (s_main_sel + 1) % MAIN_COUNT;

    /* Prev and next dimmed */
    ST7789_DrawString(8,   Y_ITEMS, s_main_labels[prev], COLOR_ITEM, COLOR_BG, &Font_8x8);
    ST7789_DrawString(170, Y_ITEMS, s_main_labels[next], COLOR_ITEM, COLOR_BG, &Font_8x8);

    /* Selected - cyan if focus is on it, white if submenu is active */
    uint16_t sel_color = (s_focus == FOCUS_HORIZONTAL) ? COLOR_SELECTED : COLOR_ACTIVE;
    ST7789_DrawString(88, Y_ITEMS, s_main_labels[s_main_sel], sel_color, COLOR_BG, &Font_8x8);

    /* Underline */
    uint16_t sel_w = strlen(s_main_labels[s_main_sel]) * Font_8x8.width;
    ST7789_FillRect(88, Y_ITEMS + 10, 80, 2, COLOR_BG);
    ST7789_FillRect(88, Y_ITEMS + 10, sel_w, 2, COLOR_INDICATOR);

    ST7789_DrawLine(5, Y_DIVIDER_B, 235, Y_DIVIDER_B, COLOR_DIVIDER);
}

/* ============================================================================
 * Draw: brightness detail
 * ============================================================================ */
static void draw_brightness_detail(void)
{
    uint16_t actual = brightness_display();
    uint8_t pct = (uint8_t)((actual * 100UL) / BRIGHTNESS_MAX);
    char buf[8];
    snprintf(buf, sizeof(buf), "%d%%", pct);

    ST7789_FillRect(0, Y_DETAIL, ST7789_WIDTH, 100, COLOR_BG);
    ST7789_DrawString(20, Y_DETAIL, "Brightness", ST7789_LIGHTGRAY, COLOR_BG, &Font_8x8);
    ST7789_DrawProgressBar(20, Y_DETAIL + 18, 160, 14, pct, ST7789_CYAN, COLOR_BG);
    ST7789_DrawString(188, Y_DETAIL + 20, buf, ST7789_WHITE, COLOR_BG, &Font_8x8);

    if (s_focus == FOCUS_BRIGHT) {
        ST7789_DrawString(20, Y_DETAIL + 42,
            "< darker    brighter >",
            ST7789_DARKGRAY, COLOR_BG, &Font_8x8);
        ST7789_DrawString(20, Y_DETAIL + 56,
            "MID = back",
            ST7789_DARKGRAY, COLOR_BG, &Font_8x8);
    }
}

/* ============================================================================
 * Draw: power submenu detail
 * ============================================================================ */
static void draw_power_detail(void)
{
    ST7789_FillRect(0, Y_DETAIL, ST7789_WIDTH, 130, COLOR_BG);
    power_mode_t cur = power_mode_get();

    for (int i = 0; i < POWER_COUNT; i++) {
        uint16_t y = Y_DETAIL + i * 22;
        uint8_t  is_sel = (s_power_sel == (power_item_t)i);

        /* Selection arrow */
        ST7789_DrawString(8, y, is_sel ? ">" : " ", COLOR_ACTIVE, COLOR_BG, &Font_8x8);

        /* Label */
        uint16_t lbl_color = is_sel ? COLOR_SELECTED : COLOR_ITEM;
        ST7789_DrawString(20, y, s_power_labels[i], lbl_color, COLOR_BG, &Font_8x8);

        /* Active indicator */
        uint8_t on = 0;
        if      (i == POWER_PERF)   on = (cur == POWER_MODE_PERF);
        else if (i == POWER_BATSAVE) on = (cur == POWER_MODE_BATSAVE);
        else if (i == POWER_BUZZER)  on = (cur == POWER_MODE_BUZZER_ONLY);

        if (i != POWER_BACK) {
            ST7789_DrawString(160, y, on ? "[ ON ]" : "[ OFF]",
                              on ? COLOR_ON : COLOR_OFF, COLOR_BG, &Font_8x8);
        }
    }
}

/* ============================================================================
 * Draw: detail area dispatcher
 * ============================================================================ */
static void draw_detail(void)
{
    ST7789_FillRect(0, Y_DETAIL, ST7789_WIDTH, 130, COLOR_BG);

    switch (s_focus) {
        case FOCUS_HORIZONTAL:
            switch (s_main_sel) {
                case MAIN_BACK:
                    ST7789_DrawString(20, Y_DETAIL,
                        "Return to main screen",
                        ST7789_LIGHTGRAY, COLOR_BG, &Font_8x8);
                    break;

                case MAIN_BRIGHT: {
                    /* Preview: show brightness bar, dimmed */
                    uint16_t actual = brightness_display();
                    uint8_t pct = (uint8_t)((actual * 100UL) / BRIGHTNESS_MAX);
                    char buf[8];
                    snprintf(buf, sizeof(buf), "%d%%", pct);
                    ST7789_DrawString(20, Y_DETAIL,
                        "Brightness", ST7789_LIGHTGRAY, COLOR_BG, &Font_8x8);
                    ST7789_DrawProgressBar(20, Y_DETAIL + 18, 160, 14,
                                          pct, COLOR_DIVIDER, COLOR_BG);
                    ST7789_DrawString(188, Y_DETAIL + 20,
                        buf, COLOR_ITEM, COLOR_BG, &Font_8x8);
                    ST7789_DrawString(20, Y_DETAIL + 42,
                        "MID to adjust",
                        ST7789_DARKGRAY, COLOR_BG, &Font_8x8);
                    break;
                }

                case MAIN_POWER: {
                    /* Preview: show power items dimmed, no selection arrow */
                    power_mode_t cur = power_mode_get();
                    ST7789_DrawString(20, Y_DETAIL,
                        "Power settings",
                        ST7789_LIGHTGRAY, COLOR_BG, &Font_8x8);
                    for (int i = 1; i < POWER_COUNT; i++) {
                        uint16_t y = Y_DETAIL + 18 + (i - 1) * 18;
                        ST7789_DrawString(20, y,
                            s_power_labels[i], COLOR_ITEM, COLOR_BG, &Font_8x8);
                        uint8_t on = 0;
                        if      (i == POWER_PERF)    on = (cur == POWER_MODE_PERF);
                        else if (i == POWER_BATSAVE)  on = (cur == POWER_MODE_BATSAVE);
                        else if (i == POWER_BUZZER)   on = (cur == POWER_MODE_BUZZER_ONLY);
                        ST7789_DrawString(160, y,
                            on ? "[ ON ]" : "[ OFF]",
                            COLOR_ITEM, COLOR_BG, &Font_8x8);
                    }
                    ST7789_DrawString(20, Y_DETAIL + 76,
                        "MID to open",
                        ST7789_DARKGRAY, COLOR_BG, &Font_8x8);
                    break;
                }

                case MAIN_GRAPH:
                    ST7789_DrawString(20, Y_DETAIL,
                        "5min dose graph",
                        ST7789_LIGHTGRAY, COLOR_BG, &Font_8x8);
                    ST7789_DrawString(20, Y_DETAIL + 16,
                        "[Coming soon]",
                        ST7789_DARKGRAY, COLOR_BG, &Font_8x8);
                    break;
                default: break;
            }
            break;

        case FOCUS_BRIGHT:
            draw_brightness_detail();
            break;

        case FOCUS_POWER:
            draw_power_detail();
            break;
    }
}

/* ============================================================================
 * Update button bar based on focus
 * ============================================================================ */
static void update_button_bar(void)
{
    switch (s_focus) {
        case FOCUS_HORIZONTAL:
            ui_draw_button_bar("<", "Select", ">");
            break;
        case FOCUS_BRIGHT:
            ui_draw_button_bar("<", "Back", ">");
            break;
        case FOCUS_POWER:
            ui_draw_button_bar("^", "Select", "v");
            break;
    }
}

/* ============================================================================
 * Screen callbacks
 * ============================================================================ */
static void menu_on_enter(void)
{
    s_focus     = FOCUS_HORIZONTAL;
    s_main_sel  = MAIN_BACK;
    s_power_sel = POWER_BACK;

    ST7789_FillScreen(COLOR_BG);

    /* Title */
    uint16_t tw = ST7789_GFXStringWidth("Settings", &FreeSansBold12pt7b);
    ST7789_DrawGFXString((ST7789_WIDTH - tw) / 2, Y_TITLE,
                         "Settings", ST7789_CYAN, COLOR_BG, &FreeSansBold12pt7b);
    ST7789_DrawLine(5, Y_DIVIDER_T, 235, Y_DIVIDER_T, COLOR_DIVIDER);

    draw_main_items();
    draw_detail();
    update_button_bar();
}

static void menu_on_data(const display_data_t *data)
{
    (void)data;
}

static screen_id_t menu_on_event(btn_event_t evt)
{
    switch (s_focus) {

        /* ------------------------------------------------------------------ */
        case FOCUS_HORIZONTAL:
            switch (evt) {
                case BTN_LEFT:
                    s_main_sel = (s_main_sel + MAIN_COUNT - 1) % MAIN_COUNT;
                    draw_main_items();
                    draw_detail();
                    break;
                case BTN_RIGHT:
                    s_main_sel = (s_main_sel + 1) % MAIN_COUNT;
                    draw_main_items();
                    draw_detail();
                    break;
                case BTN_MID:
                    if (s_main_sel == MAIN_BACK)  return SCREEN_MAIN;
                    if (s_main_sel == MAIN_GRAPH) return SCREEN_MAIN;
                    if (s_main_sel == MAIN_BRIGHT) {
                        s_focus = FOCUS_BRIGHT;
                        draw_main_items();
                        draw_detail();
                        update_button_bar();
                    }
                    if (s_main_sel == MAIN_POWER) {
                        s_focus     = FOCUS_POWER;
                        s_power_sel = POWER_BACK;
                        draw_main_items();
                        draw_detail();
                        update_button_bar();
                    }
                    break;
                default: break;
            }
            break;

        /* ------------------------------------------------------------------ */
        case FOCUS_BRIGHT:
            switch (evt) {
                case BTN_LEFT:
                case BTN_LEFT_LONG:
                    brightness_change(-1);
                    draw_brightness_detail();
                    break;
                case BTN_RIGHT:
                case BTN_RIGHT_LONG:
                    brightness_change(+1);
                    draw_brightness_detail();
                    break;
                case BTN_MID:
                    /* Back to horizontal nav */
                    s_focus = FOCUS_HORIZONTAL;
                    draw_main_items();
                    draw_detail();
                    update_button_bar();
                    break;
                default: break;
            }
            break;

        /* ------------------------------------------------------------------ */
        case FOCUS_POWER:
            switch (evt) {
                case BTN_LEFT:  /* up */
                    s_power_sel = (s_power_sel + POWER_COUNT - 1) % POWER_COUNT;
                    draw_power_detail();
                    break;
                case BTN_RIGHT:  /* down */
                    s_power_sel = (s_power_sel + 1) % POWER_COUNT;
                    draw_power_detail();
                    break;
                case BTN_MID:
                    if (s_power_sel == POWER_BACK) {
                        s_focus = FOCUS_HORIZONTAL;
                        draw_main_items();
                        draw_detail();
                        update_button_bar();
                    } else if (s_power_sel == POWER_PERF) {
                        /* Toggle PERF - if already PERF go back to DEFAULT */
                        if (power_mode_get() == POWER_MODE_PERF)
                            power_mode_set(POWER_MODE_DEFAULT);
                        else
                            power_mode_set(POWER_MODE_PERF);
                        draw_power_detail();
                    } else if (s_power_sel == POWER_BATSAVE) {
                        /* Toggle BATSAVE */
                        if (power_mode_get() == POWER_MODE_BATSAVE) {
                            /* Restore brightness from before BatSave */
                            s_brightness = s_brightness_saved;
                            brightness_apply();
                            power_mode_set(POWER_MODE_DEFAULT);
                        } else {
                            /* Save current brightness, switch to BatSave level */
                            s_brightness_saved = s_brightness;
                            s_brightness = BRIGHTNESS_BATSAVE;
                            brightness_apply();
                            power_mode_set(POWER_MODE_BATSAVE);
                        }
                        draw_power_detail();
                    } else if (s_power_sel == POWER_BUZZER) {
                        /* Toggle BUZZER_ONLY */
                        if (power_mode_get() == POWER_MODE_BUZZER_ONLY)
                            power_mode_set(POWER_MODE_DEFAULT);
                        else
                            power_mode_set(POWER_MODE_BUZZER_ONLY);
                        draw_power_detail();
                    }
                    break;
                default: break;
            }
            break;
    }

    return SCREEN_MENU;
}

const screen_t screen_menu = {
    .on_enter = menu_on_enter,
    .on_data  = menu_on_data,
    .on_event = menu_on_event,
};
