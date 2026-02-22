/**
  ******************************************************************************
  * @file    screen_splash.c
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Splash Screen (shown 2s on startup)
  ******************************************************************************
  */

#include "screen_splash.h"
#include "screen_main.h"
#include "st7789.h"
#include "gfx_fonts.h"
#include "tx_api.h"

#define SPLASH_DURATION_TICKS  250   /* 200 x 10ms = 2s */

static uint32_t s_ticks = 0;

static void splash_on_enter(void)
{
    s_ticks = 0;
    ST7789_FillScreen(ST7789_BLACK);

    /* "RADONE" centered, large */
    uint16_t w = ST7789_GFXStringWidth("RADONE", &FreeSansBold18pt7b);
    uint16_t x = (ST7789_WIDTH - w) / 2;
    ST7789_DrawGFXString(x,     120, "RAD", ST7789_CYAN,  ST7789_BLACK,
                         &FreeSansBold18pt7b);
    ST7789_DrawGFXString(x + ST7789_GFXStringWidth("RAD", &FreeSansBold18pt7b),
                         120, "ONE", ST7789_WHITE, ST7789_BLACK,
                         &FreeSansBold18pt7b);

    /* Version string */
    uint16_t vw = ST7789_GFXStringWidth("v0.1", &FreeSans12pt7b);
    ST7789_DrawGFXString((ST7789_WIDTH - vw) / 2, 148,
                         "v0.1", ST7789_DARKGRAY, ST7789_BLACK,
                         &FreeSans12pt7b);
}

static void splash_on_data(const display_data_t *data)
{
    (void)data;
    /* Auto-advance to main screen after SPLASH_DURATION_TICKS.
     * Called once per second from display task via data update. */
    if (++s_ticks >= 2)
        screen_manager_switch(SCREEN_MAIN);
}

static screen_id_t splash_on_event(btn_event_t evt)
{
    /* Any button press skips the splash */
    if (evt != BTN_NONE)
        return SCREEN_MAIN;
    return SCREEN_SPLASH;
}

const screen_t screen_splash = {
    .on_enter = splash_on_enter,
    .on_data  = splash_on_data,
    .on_event = splash_on_event,
};
