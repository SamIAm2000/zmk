/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <zephyr/kernel.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <lvgl.h>
#include <zmk/display/widgets/bongo_cat.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);
static lv_anim_t idle_anim;
static lv_anim_t tap_anim;

LV_IMG_DECLARE(idle_img1);
LV_IMG_DECLARE(idle_img2);
LV_IMG_DECLARE(idle_img3);
LV_IMG_DECLARE(idle_img4);
LV_IMG_DECLARE(idle_img5);
LV_IMG_DECLARE(fast_img1);
LV_IMG_DECLARE(fast_img2);

static const void *idle_images[] = {&idle_img1, &idle_img2, &idle_img3, &idle_img4, &idle_img5};

static const void *tap_images[] = {&fast_img1, &fast_img2};

#define IDLE_FRAMES 5
#define TAP_FRAMES 2
#define IDLE_ANIM_TIME 1000 // 1 second for full idle cycle
#define TAP_ANIM_TIME 100   // 100ms for tap animation
#define IDLE_TIMEOUT_MS 500 // Return to idle after 500ms of no keypresses

struct bongo_cat_state {
    bool key_pressed;
    uint32_t last_tap;
};

static void set_idle_frame(void *var, int32_t val) {
    LOG_DBG("BONGO: Idle animation frame: %d", val);
    lv_obj_t *img = (lv_obj_t *)var;
    int frame = val % IDLE_FRAMES;
    lv_img_set_src(img, idle_images[frame]);
}

static void set_tap_frame(void *var, int32_t val) {
    LOG_DBG("BONGO: Tap animation frame: %d", val);
    lv_obj_t *img = (lv_obj_t *)var;
    lv_img_set_src(img, tap_images[val % TAP_FRAMES]);
}

static void start_idle_animation(lv_obj_t *obj) {
    LOG_DBG("BONGO: Starting idle animation");
    lv_anim_del(obj, set_tap_frame);

    lv_anim_init(&idle_anim);
    lv_anim_set_var(&idle_anim, obj);
    lv_anim_set_values(&idle_anim, 0, IDLE_FRAMES - 1);
    lv_anim_set_time(&idle_anim, IDLE_ANIM_TIME);
    lv_anim_set_exec_cb(&idle_anim, set_idle_frame);
    lv_anim_set_repeat_count(&idle_anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&idle_anim);
}

static void play_tap_animation(lv_obj_t *obj) {
    LOG_DBG("BONGO: Playing tap animation");
    lv_anim_del(obj, set_idle_frame);

    static uint8_t current_frame = 0;
    current_frame = (current_frame + 1) % TAP_FRAMES;
    lv_img_set_src(obj, tap_images[current_frame]);
}

static void update_bongo_cat_anim(struct zmk_widget_bongo_cat *widget,
                                  struct bongo_cat_state state) {
    if (!widget || !widget->obj) {
        LOG_ERR("BONGO: Widget or object is NULL!");
        return;
    }

    uint32_t now = k_uptime_get_32();
    uint32_t time_since_last_tap = now - state.last_tap;

    if (state.key_pressed) {
        // Play tap animation on keypress
        play_tap_animation(widget->obj);
    } else if (time_since_last_tap >= IDLE_TIMEOUT_MS) {
        // Return to idle animation after timeout
        start_idle_animation(widget->obj);
    }
}

static struct bongo_cat_state bongo_cat_get_state(const zmk_event_t *eh) {
    static struct bongo_cat_state state = {.key_pressed = false, .last_tap = 0};

    const struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    if (ev != NULL && ev->state) { // Only update on key press, not release
        state.key_pressed = true;
        state.last_tap = k_uptime_get_32();
    } else {
        state.key_pressed = false;
    }

    return state;
}

static void bongo_cat_update_cb(struct bongo_cat_state state) {
    struct zmk_widget_bongo_cat *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { update_bongo_cat_anim(widget, state); }
}

int zmk_widget_bongo_cat_init(struct zmk_widget_bongo_cat *widget, lv_obj_t *parent) {
    LOG_DBG("BONGO: === INITIALIZATION START ===");

    if (!widget || !parent) {
        LOG_ERR("BONGO: Invalid widget or parent!");
        return -1;
    }

    widget->obj = lv_img_create(parent);
    if (widget->obj == NULL) {
        LOG_ERR("BONGO: Failed to create LVGL image object");
        return -1;
    }

    LOG_DBG("BONGO: Image object created");

    // Start with idle animation
    start_idle_animation(widget->obj);

    sys_slist_append(&widgets, &widget->node);

    LOG_DBG("BONGO: === INITIALIZATION COMPLETE ===");
    return 0;
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_bongo_cat, struct bongo_cat_state, bongo_cat_update_cb,
                            bongo_cat_get_state)
ZMK_SUBSCRIPTION(widget_bongo_cat, zmk_keycode_state_changed);

lv_obj_t *zmk_widget_bongo_cat_obj(struct zmk_widget_bongo_cat *widget) { return widget->obj; }
