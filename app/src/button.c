#include <zephyr/kernel.h>
#include <zephyr/input/input.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <esp_sleep.h>
#include "button.h"

LOG_MODULE_REGISTER(button, LOG_LEVEL_INF);

/* Long enough for the cat to show the full tap frame (CAT_HIT_MS). */
#define WAKEUP_RELEASE_DELAY_MS 150

static const struct gpio_dt_spec key_left = GPIO_DT_SPEC_GET(DT_NODELABEL(key1), gpios);
static const struct gpio_dt_spec key_right = GPIO_DT_SPEC_GET(DT_NODELABEL(key2), gpios);

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

static button_event_t wakeup_release_evt;

static void wakeup_release_work_cb(struct k_work *work)
{
    k_msgq_put(&button_msgq, &wakeup_release_evt, K_NO_WAIT);
}

static K_WORK_DELAYABLE_DEFINE(wakeup_release_work, wakeup_release_work_cb);

void button_replay_wakeup(void)
{
    uint64_t pin_mask = 0;
    const struct gpio_dt_spec *key;
    button_event_t press;

    if (!(esp_sleep_get_wakeup_causes() & BIT(ESP_SLEEP_WAKEUP_GPIO))) {
        return;
    }

#if defined(SOC_GPIO_SUPPORT_DEEPSLEEP_WAKEUP)
    /* On ESP32-C3, wakeup-capable GPIO n maps to bit n. */
    pin_mask = esp_sleep_get_gpio_wakeup_status();
#endif

    if (pin_mask & BIT64(key_left.pin)) {
        key = &key_left;
        press = BUTTON_L_PRESSED;
        wakeup_release_evt = BUTTON_L_RELEASED;
    }
    else if (pin_mask & BIT64(key_right.pin)) {
        key = &key_right;
        press = BUTTON_R_PRESSED;
        wakeup_release_evt = BUTTON_R_RELEASED;
    }
    else {
        LOG_DBG("Woken by GPIO, but not by a known key (mask 0x%llx)", pin_mask);
        return;
    }

    LOG_INF("Woken by %s key", key == &key_left ? "left" : "right");
    k_msgq_put(&button_msgq, &press, K_NO_WAIT);

    /*
     * gpio-keys takes the pin level at boot as its initial state and does not
     * report it. If the key is still held, the driver reports the real release
     * later; otherwise it was already released during boot, so fake one.
     */
    if (gpio_pin_get_dt(key) <= 0) {
        k_work_schedule(&wakeup_release_work, K_MSEC(WAKEUP_RELEASE_DELAY_MS));
    }
}
