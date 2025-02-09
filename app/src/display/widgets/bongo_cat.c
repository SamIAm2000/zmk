/*
 * Copyright (c) 2021 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <zephyr/kernel.h>
#include <zmk/event_manager.h>
#include <zmk/events/wpm_state_changed.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <lvgl.h>
#include <zmk/display/widgets/bongo_cat.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);
static lv_anim_t widget_anim;

LV_IMG_DECLARE(idle_img1);
LV_IMG_DECLARE(fast_img1);
LV_IMG_DECLARE(fast_img2);

static const void *idle_images[] = {&idle_img1};

static const void *typing_images[] = {&fast_img1, &fast_img2};

struct bongo_cat_state {
    int wpm;
    const void **current_images;
    int current_frame;
    int num_frames;
};

static void set_img_src(void *var, int32_t val) {
    LOG_ERR("BONGO: Animation frame callback: %d", val); // Using ERR for visibility
    lv_obj_t *img = (lv_obj_t *)var;
    struct bongo_cat_state *state = (struct bongo_cat_state *)lv_obj_get_user_data(img);
    if (state && state->current_images) {
        int frame = val % state->num_frames;
        LOG_ERR("BONGO: Setting frame %d of %d", frame, state->num_frames);
        lv_img_set_src(img, state->current_images[frame]);
    }
}

static void update_bongo_cat_anim(struct zmk_widget_bongo_cat *widget,
                                  struct bongo_cat_state state) {
    LOG_ERR("BONGO: Updating animation, WPM: %d", state.wpm);

    if (!widget || !widget->obj) {
        LOG_ERR("BONGO: Widget or object is NULL!");
        return;
    }

    // Stop any existing animation
    lv_anim_del(widget->obj, set_img_src);

    // Set up new animation
    if (state.wpm == 0) {
        LOG_ERR("BONGO: Setting idle animation");
        state.current_images = idle_images;
        state.num_frames = 1;
        lv_img_set_src(widget->obj, idle_images[0]);
    } else {
        LOG_ERR("BONGO: Setting typing animation");
        state.current_images = typing_images;
        state.num_frames = 2;

        lv_anim_init(&widget_anim);
        lv_anim_set_var(&widget_anim, widget->obj);
        lv_anim_set_values(&widget_anim, 0, 1);
        lv_anim_set_time(&widget_anim, 200);
        lv_anim_set_exec_cb(&widget_anim, set_img_src);
        lv_anim_set_repeat_count(&widget_anim, LV_ANIM_REPEAT_INFINITE);

        lv_obj_set_user_data(widget->obj, &state);
        lv_anim_start(&widget_anim);
    }
}

static struct bongo_cat_state bongo_cat_get_state(const zmk_event_t *eh) {
    struct bongo_cat_state state = {
        .wpm = zmk_wpm_get_state(), .current_images = NULL, .current_frame = 0, .num_frames = 0};
    LOG_ERR("BONGO: Current WPM: %d", state.wpm);
    return state;
}

static void bongo_cat_update_cb(struct bongo_cat_state state) {
    LOG_ERR("BONGO: Update callback triggered");
    struct zmk_widget_bongo_cat *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { update_bongo_cat_anim(widget, state); }
}

int zmk_widget_bongo_cat_init(struct zmk_widget_bongo_cat *widget, lv_obj_t *parent) {
    LOG_ERR("BONGO: === INITIALIZATION START ===");

    if (!widget || !parent) {
        LOG_ERR("BONGO: Invalid widget or parent!");
        return -1;
    }

    widget->obj = lv_img_create(parent);
    if (widget->obj == NULL) {
        LOG_ERR("BONGO: Failed to create LVGL image object");
        return -1;
    }

    LOG_ERR("BONGO: Image object created");

    struct bongo_cat_state initial_state = bongo_cat_get_state(NULL);
    update_bongo_cat_anim(widget, initial_state);

    sys_slist_append(&widgets, &widget->node);

    LOG_ERR("BONGO: === INITIALIZATION COMPLETE ===");
    return 0;
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_bongo_cat, struct bongo_cat_state, bongo_cat_update_cb,
                            bongo_cat_get_state)

ZMK_SUBSCRIPTION(widget_bongo_cat, zmk_wpm_state_changed);

lv_obj_t *zmk_widget_bongo_cat_obj(struct zmk_widget_bongo_cat *widget) { return widget->obj; }
