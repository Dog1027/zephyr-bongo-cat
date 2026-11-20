#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <lvgl.h>
#include "button.h"
#include "heart.h"
#include "cat.h"

typedef enum {
    CAT_IDLE,
    CAT_PREP,
    CAT_TAP_R,
    CAT_TAP_RD,
    CAT_TAP_L,
    CAT_TAP_LD,
    CAT_TAP_B,
    CAT_TAP_BD,
    CAT_TAP_BRD,
    CAT_TAP_BLD,
} cat_state_t;

#define CAT_HIT_MS (100UL)
#define CAT_PREP_MS (1200UL)
#define HEART_POS_RIGHT_X (70)
#define HEART_POS_RIGHT_Y (22)
#define HEART_POS_LEFT_X (28)
#define HEART_POS_LEFT_Y (28)
#define HEART_POS_RANDOM_OFFSET_X (30)
#define HEART_POS_RANDOM_OFFSET_Y (12)
#define GET_HEART_POS_RIGHT_X() (HEART_POS_RIGHT_X + sys_rand8_get() * HEART_POS_RANDOM_OFFSET_X / 255)
#define GET_HEART_POS_RIGHT_Y() (HEART_POS_RIGHT_Y + sys_rand8_get() * HEART_POS_RANDOM_OFFSET_Y / 255)
#define GET_HEART_POS_LEFT_X() (HEART_POS_LEFT_X + sys_rand8_get() * HEART_POS_RANDOM_OFFSET_X / 255)
#define GET_HEART_POS_LEFT_Y() (HEART_POS_LEFT_Y + sys_rand8_get() * HEART_POS_RANDOM_OFFSET_Y / 255)
#define GET_IMG(name) extern const lv_image_dsc_t name;

K_TIMER_DEFINE(cat_state_timer, NULL, NULL);

GET_IMG(cat_idle_1_img);
GET_IMG(cat_idle_2_img);
GET_IMG(cat_idle_3_img);
GET_IMG(cat_idle_4_img);
GET_IMG(cat_idle_5_img);
GET_IMG(cat_prep_img);
GET_IMG(cat_tap_r_img);
GET_IMG(cat_tap_rd_img);
GET_IMG(cat_tap_l_img);
GET_IMG(cat_tap_ld_img);
GET_IMG(cat_tap_b_img);
GET_IMG(cat_tap_bd_img);
GET_IMG(cat_tap_brd_img);
GET_IMG(cat_tap_bld_img);

static const lv_img_dsc_t *cat_idle_img[] = {
    &cat_idle_1_img,
    &cat_idle_2_img,
    &cat_idle_3_img,
    &cat_idle_4_img,
    &cat_idle_5_img
};
static lv_obj_t *cat_img;
static lv_timer_t *cat_anim_timer;
static cat_state_t cat_state = CAT_IDLE;

static void cat_anim_timer_cb(lv_timer_t *timer)
{
    static uint8_t curr_frame = 0;
    curr_frame = (curr_frame + 1) % ARRAY_SIZE(cat_idle_img);
    lv_image_set_src(cat_img, cat_idle_img[curr_frame]);
}

static void update_cat_img(cat_state_t state)
{
    switch (state) {
    case CAT_IDLE:
        lv_timer_resume(cat_anim_timer);
        break;
    case CAT_PREP:
        lv_timer_pause(cat_anim_timer);
        lv_image_set_src(cat_img, &cat_prep_img);
        break;
    case CAT_TAP_R:
        lv_timer_pause(cat_anim_timer);
        lv_image_set_src(cat_img, &cat_tap_r_img);
        break;
    case CAT_TAP_RD:
        lv_timer_pause(cat_anim_timer);
        lv_image_set_src(cat_img, &cat_tap_rd_img);
        break;
    case CAT_TAP_L:
        lv_timer_pause(cat_anim_timer);
        lv_image_set_src(cat_img, &cat_tap_l_img);
        break;
    case CAT_TAP_LD:
        lv_timer_pause(cat_anim_timer);
        lv_image_set_src(cat_img, &cat_tap_ld_img);
        break;
    case CAT_TAP_B:
        lv_timer_pause(cat_anim_timer);
        lv_image_set_src(cat_img, &cat_tap_b_img);
        break;
    case CAT_TAP_BD:
        lv_timer_pause(cat_anim_timer);
        lv_image_set_src(cat_img, &cat_tap_bd_img);
        break;
    case CAT_TAP_BRD:
        lv_timer_pause(cat_anim_timer);
        lv_image_set_src(cat_img, &cat_tap_brd_img);
        break;
    case CAT_TAP_BLD:
        lv_timer_pause(cat_anim_timer);
        lv_image_set_src(cat_img, &cat_tap_bld_img);
        break;
    }
}

static cat_state_t update_cat_state(cat_state_t state) {
    static int64_t last = 0;
    int64_t now = k_uptime_get();
    button_event_t evt;
    cat_state_t next_state = state;

    if (now - last >= CAT_HIT_MS) {
        switch (next_state) {
        case CAT_TAP_L:
            last = now;
            next_state = CAT_TAP_LD;
            break;
        case CAT_TAP_R:
            last = now;
            next_state = CAT_TAP_RD;
            break;
        case CAT_TAP_B:
        case CAT_TAP_BRD:
        case CAT_TAP_BLD:
            last = now;
            next_state = CAT_TAP_BD;
            break;
        default:
            break;
        }
    }

    if (now - last >= CAT_PREP_MS) {
        if (next_state == CAT_PREP) {
            last = now;
            next_state = CAT_IDLE;
        }
    }

    while (button_get(&evt) == 0) {
        switch (next_state) {
        case CAT_IDLE:
        case CAT_PREP:
            switch (evt) {
            case BUTTON_R_PRESSED:
                next_state = CAT_TAP_R;
                break;
            case BUTTON_L_PRESSED:
                next_state = CAT_TAP_L;
                break;
            default:
                break;
            }
            break;
        case CAT_TAP_R:
            switch (evt) {
            case BUTTON_R_RELEASED:
                next_state = CAT_PREP;
                break;
            case BUTTON_L_PRESSED:
                next_state = CAT_TAP_B;
                break;
            default:
                break;
            }
            break;
        case CAT_TAP_RD:
            switch (evt) {
            case BUTTON_L_PRESSED:
                next_state = CAT_TAP_BRD;
                break;
            case BUTTON_R_RELEASED:
                next_state = CAT_PREP;
                break;
            default:
                break;
            }
            break;
        case CAT_TAP_L:
            switch (evt) {
            case BUTTON_R_PRESSED:
                next_state = CAT_TAP_B;
                break;
            case BUTTON_L_RELEASED:
                next_state = CAT_PREP;
                break;
            default:
                break;
            }
            break;
        case CAT_TAP_LD:
            switch (evt) {
            case BUTTON_R_PRESSED:
                next_state = CAT_TAP_BLD;
                break;
            case BUTTON_L_RELEASED:
                next_state = CAT_PREP;
                break;
            default:
                break;
            }
            break;
        case CAT_TAP_B:
        case CAT_TAP_BRD:
        case CAT_TAP_BLD:
            switch (evt) {
            case BUTTON_L_PRESSED:
            case BUTTON_R_PRESSED:
                next_state = CAT_PREP;
                break;
            default:
                break;
            }
            break;
        case CAT_TAP_BD:
            switch (evt) {
            case BUTTON_L_RELEASED:
                next_state = CAT_TAP_RD;
                break;
            case BUTTON_R_RELEASED:
                next_state = CAT_TAP_LD;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }

    return next_state;
}

void cat_init(void)
{
    cat_img = lv_image_create(lv_screen_active());
    lv_obj_set_style_image_recolor(cat_img, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(cat_img, LV_ALIGN_CENTER, 0, 0);
    lv_image_set_src(cat_img, cat_idle_img[0]);
    cat_anim_timer = lv_timer_create(cat_anim_timer_cb, 100, NULL);
    lv_timer_resume(cat_anim_timer);
}

int cat_update(void)
{
    cat_state_t new_cat_state = update_cat_state(cat_state);
    if (new_cat_state != cat_state) {
        update_cat_img(new_cat_state);
        switch (new_cat_state) {
        case CAT_TAP_R:
        case CAT_TAP_BLD:
            heart_append(GET_HEART_POS_RIGHT_X(), GET_HEART_POS_RIGHT_Y());
            break;
        case CAT_TAP_L:
        case CAT_TAP_BRD:
            heart_append(GET_HEART_POS_LEFT_X(), GET_HEART_POS_LEFT_Y());
            break;
        case CAT_TAP_B:
            heart_append(GET_HEART_POS_RIGHT_X(), GET_HEART_POS_RIGHT_Y());
            heart_append(GET_HEART_POS_LEFT_X(), GET_HEART_POS_LEFT_Y());
            break;
        default:
            break;
        }
        cat_state = new_cat_state;
        return 1; // Indicate that the cat state has changed
    }
    return 0; // Indicate that the cat state has not changed
}
