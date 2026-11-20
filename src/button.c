#include <zephyr/kernel.h>
#include <zephyr/input/input.h>
#include "button.h"

K_MSGQ_DEFINE(button_msgq, sizeof(button_event_t), 10, 4);

static void button_cb(struct input_event *p_evt, void *user_data)
{
    if (!p_evt->sync) {
        return;
    }

    if (p_evt->type == INPUT_EV_KEY) {
        button_event_t b_evt;
        switch (p_evt->code) {
        case INPUT_KEY_RIGHT:
            if (p_evt->value != 0) {
                b_evt = BUTTON_R_PRESSED;
            }
            else {
                b_evt = BUTTON_R_RELEASED;
            }
            break;
        case INPUT_KEY_LEFT:
            if (p_evt->value != 0) {
                b_evt = BUTTON_L_PRESSED;
            }
            else {
                b_evt = BUTTON_L_RELEASED;
            }
            break;
        default:
            return; // Ignore other keys
        }

        k_msgq_put(&button_msgq, &b_evt, K_NO_WAIT);
    }
}

INPUT_CALLBACK_DEFINE(NULL, button_cb, NULL);

int button_get(button_event_t *p_evt)
{
    return k_msgq_get(&button_msgq, p_evt, K_NO_WAIT);
}
