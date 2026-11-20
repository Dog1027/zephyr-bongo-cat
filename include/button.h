#pragma once

#include <stdint.h>

typedef enum {
    BUTTON_R_PRESSED,
    BUTTON_R_RELEASED,
    BUTTON_L_PRESSED,
    BUTTON_L_RELEASED,
} button_event_t;

int button_get(button_event_t *evt);

/*
 * If the system just woke from deep sleep because a key was pressed,
 * queue that press so it counts as a tap.
 */
void button_replay_wakeup(void);
