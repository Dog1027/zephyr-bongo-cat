#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/input/input.h>
#include <zephyr/sys/poweroff.h>
#include <zephyr/drivers/display.h>
#include <lvgl.h>
#include "button.h"
#include "cat.h"
#include "heart.h"
#include "storage.h"

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

/* How long the "Zzz" hint stays on screen before powering off. */
#define SLEEP_HINT_MS 1000

static lv_obj_t *heart_count_label;

static void update_label(void)
{
    static uint32_t shown_count = 0;
    uint32_t count = heart_get_count();

    if (count != shown_count) {
        shown_count = count;
        lv_label_set_text_fmt(heart_count_label, "%u", count);
    }
}

static void init(void)
{
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), LV_PART_MAIN);
    heart_count_label = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(heart_count_label, &lv_font_unscii_8, LV_PART_MAIN);
    lv_obj_set_style_text_color(heart_count_label, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text_fmt(heart_count_label, "%u", heart_get_count());
}

static void restore_count(void)
{
    uint32_t count;

    if (storage_init() == 0 && storage_load_count(&count) == 0) {
        heart_set_count(count);
        LOG_INF("Restored count: %u", count);
    }
}

static void show_sleep_hint(void)
{
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(label, &lv_font_unscii_8, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_label_set_text(label, "Zzz");
    lv_obj_align(label, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_refr_now(NULL);
    k_msleep(SLEEP_HINT_MS);
}

static void power_off(void)
{
    const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    int ret;

    ret = storage_save_count(heart_get_count());
    if (ret < 0) {
        LOG_WRN("Failed to save count (%d)", ret);
    }

    show_sleep_hint();

    if (device_is_ready(display_dev)) {
        ret = display_blanking_on(display_dev);
        if (ret < 0) {
            LOG_WRN("Failed to turn on display blanking (%d)", ret);
        }
    }
    k_msleep(100); // Give some time for the display to blank before powering off
    sys_poweroff();
}

int main(void)
{
    int64_t last = 0;
    int64_t last_activity = 0;
    uint32_t sleep_ms = 0;

    restore_count();
    init();
    cat_init();
    button_replay_wakeup();
    LOG_INF("Starting...");

    while (1) {
        int64_t now = k_uptime_get();
        if (now - last >= CONFIG_APP_HEARTBEAT_INTERVAL_MS) {
            last = now;
            LOG_DBG("ALIVE");
        }
        if (now - last_activity >= CONFIG_APP_INACTIVITY_SLEEP_MS) {
            power_off();
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
