#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/input/input.h>
#include <zephyr/sys/poweroff.h>
#include <zephyr/drivers/display.h>
#include <lvgl.h>
#include "cat.h"
#include "heart.h"

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

#define INACTIVITY_SLEEP_MS (5UL * 1000UL)

static lv_obj_t *heart_count_label;

static void update_label(void)
{
    lv_label_set_text_fmt(heart_count_label, "%d", heart_get_count());
}

static void init(void)
{
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), LV_PART_MAIN);
    heart_count_label = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(heart_count_label, &lv_font_unscii_8, LV_PART_MAIN);
    lv_obj_set_style_text_color(heart_count_label, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(heart_count_label, "0");
}

int main(void)
{
    int64_t last = 0;
    int64_t last_activity = 0;
    uint32_t sleep_ms = 0;

    init();
    cat_init();
    LOG_INF("Starting...");

    while (1) {
        int64_t now = k_uptime_get();
        if (now - last >= 1000) {
            last = now;
            LOG_INF("ALIVE");
        }
        if (now - last_activity >= INACTIVITY_SLEEP_MS) {
            const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
            if (device_is_ready(display_dev)) {
                int ret = display_blanking_on(display_dev);
                if (ret < 0) {
                    LOG_INF("Failed to turn on display blanking");
                }
            }
            k_msleep(100); // Give some time for the display to blank before powering off
            sys_poweroff();
        }

        if (cat_update()) {
            last_activity = k_uptime_get();
        }
        heart_update();
        update_label();

        sleep_ms = lv_timer_handler();
        if (sleep_ms == LV_NO_TIMER_READY) {
            sleep_ms = 1;
        }
        k_msleep(sleep_ms);
    }

    return 0;
}
