/**
  ******************************************************************************
  * @file    screen_manager.c
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Screen Manager
  ******************************************************************************
  */

#include "screen_manager.h"
#include "screen_splash.h"
#include "screen_main.h"
#include "screen_menu.h"

/* Active screen table */
static const screen_t *s_screens[] = {
    [SCREEN_SPLASH] = &screen_splash,
    [SCREEN_MAIN]   = &screen_main,
    [SCREEN_MENU]   = &screen_menu,
};

static screen_id_t s_active = SCREEN_SPLASH;

void screen_manager_init(void)
{
    s_active = SCREEN_SPLASH;
    if (s_screens[s_active]->on_enter)
        s_screens[s_active]->on_enter();
}

void screen_manager_data(const display_data_t *data)
{
    if (s_screens[s_active]->on_data)
        s_screens[s_active]->on_data(data);
}

screen_id_t screen_manager_event(btn_event_t evt)
{
    if (s_screens[s_active]->on_event) {
        screen_id_t next = s_screens[s_active]->on_event(evt);
        if (next != s_active)
            screen_manager_switch(next);
    }
    return s_active;
}

void screen_manager_switch(screen_id_t id)
{
    if (id >= sizeof(s_screens) / sizeof(s_screens[0])) return;
    s_active = id;
    if (s_screens[s_active]->on_enter)
        s_screens[s_active]->on_enter();
}
