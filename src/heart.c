#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/slist.h>
#include <lvgl.h>
#include "heart.h"

LOG_MODULE_REGISTER(heart, LOG_LEVEL_INF);

#define HEART_INTERVAL_MS 60
/* Heart style 0-2 have 13 frames; style 3 only has 10 in bitmap.c. */
#define HEART_TYPE 0
#define HEART_IMG(num) HEART_IMG_1(HEART_TYPE, num)
#define HEART_IMG_1(type, num) HEART_IMG_2(type, num)
#define HEART_IMG_2(type, num) heart_##type##_##num##_img
#define GET_IMG(name) extern const lv_image_dsc_t name;

GET_IMG(HEART_IMG(1));
GET_IMG(HEART_IMG(2));
GET_IMG(HEART_IMG(3));
GET_IMG(HEART_IMG(4));
GET_IMG(HEART_IMG(5));
GET_IMG(HEART_IMG(6));
GET_IMG(HEART_IMG(7));
GET_IMG(HEART_IMG(8));
GET_IMG(HEART_IMG(9));
GET_IMG(HEART_IMG(10));
GET_IMG(HEART_IMG(11));
GET_IMG(HEART_IMG(12));
GET_IMG(HEART_IMG(13));

static const lv_image_dsc_t *heart_img[] = {
    &HEART_IMG(1),
    &HEART_IMG(2),
    &HEART_IMG(3),
    &HEART_IMG(4),
    &HEART_IMG(5),
    &HEART_IMG(6),
    &HEART_IMG(7),
    &HEART_IMG(8),
    &HEART_IMG(9),
    &HEART_IMG(10),
    &HEART_IMG(11),
    &HEART_IMG(12),
    &HEART_IMG(13)
};

typedef struct {
    int64_t stamp;
    lv_obj_t *img;
    uint8_t frame;
} heart_t;

typedef struct {
    sys_snode_t node;
    heart_t h;
} heart_node_t;

static sys_slist_t heart_list = SYS_SLIST_STATIC_INIT(&heart_list);
static uint32_t heart_count = 0;

void heart_append(uint32_t x, uint32_t y)
{
    /* Count the tap even if there is no memory left to animate it. */
    heart_count++;

    heart_node_t *node = k_malloc(sizeof(heart_node_t));
    if (node == NULL) {
        LOG_WRN("Out of memory, skipping heart animation");
        return;
    }

    node->h.stamp = k_uptime_get();
    node->h.frame = 0;
    node->h.img = lv_image_create(lv_screen_active());
    lv_obj_set_style_image_recolor(node->h.img, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_pos(node->h.img, x, y);
    lv_image_set_src(node->h.img, heart_img[0]);
    sys_slist_append(&heart_list, &node->node);
}

void heart_update(void)
{
    heart_node_t *node, *tmp;
    sys_snode_t *prev = NULL;
    int64_t now = k_uptime_get();

    SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&heart_list, node, tmp, node) {
        int64_t elapse = now - node->h.stamp;
        if (elapse >= HEART_INTERVAL_MS * ARRAY_SIZE(heart_img)) {
            sys_slist_remove(&heart_list, prev, &node->node);
            lv_obj_delete(node->h.img);
            k_free(node);
        }
        else {
            uint8_t frame = elapse / HEART_INTERVAL_MS;
            if (frame != node->h.frame) {
                node->h.frame = frame;
                lv_image_set_src(node->h.img, heart_img[frame]);
            }
            prev = &node->node;
        }
    }
}

uint32_t heart_get_count(void)
{
    return heart_count;
}
