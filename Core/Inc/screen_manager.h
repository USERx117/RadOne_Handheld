/**
  ******************************************************************************
  * @file    screen_manager.h
  * @author  Florian Gotschim
  * @brief   RadOne Handheld - Screen Manager
  *
  *          Central dispatcher between Button events and active screen.
  *          Each screen implements 3 callbacks:
  *            on_enter()  - draw static layout
  *            on_data()   - update dynamic values (display_data_t)
  *            on_event()  - handle btn_event_t, return next screen ID
  ******************************************************************************
  */

#ifndef INC_SCREEN_MANAGER_H_
#define INC_SCREEN_MANAGER_H_

#include "display.h"
#include "peripherals.h"

/* ============================================================================
 * Screen IDs
 * ============================================================================ */
typedef enum {
    SCREEN_SPLASH = 0,
    SCREEN_MAIN,
    SCREEN_MENU,
} screen_id_t;

/* ============================================================================
 * Screen interface - each screen implements these
 * ============================================================================ */
typedef struct {
    void         (*on_enter)(void);
    void         (*on_data) (const display_data_t *data);
    screen_id_t  (*on_event)(btn_event_t evt);
} screen_t;

/* ============================================================================
 * Public API - called from display_task_entry
 * ============================================================================ */
void        screen_manager_init  (void);
void        screen_manager_data  (const display_data_t *data);
screen_id_t screen_manager_event (btn_event_t evt);
void        screen_manager_switch(screen_id_t id);

#endif /* INC_SCREEN_MANAGER_H_ */
