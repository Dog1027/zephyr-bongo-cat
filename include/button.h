#pragma once

#include <stdint.h>

typedef enum {
    BUTTON_R_PRESSED,
    BUTTON_R_RELEASED,
    BUTTON_L_PRESSED,
    BUTTON_L_RELEASED,
} button_event_t;

int button_get(button_event_t *evt);
