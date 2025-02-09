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

// Persistent animation state
static struct {
    const void **current_images;
    int num_frames;
    bool is_typing;
} anim_state = {.current_images = NULL, .num_frames = 0, .is_typing = false};

static const void *idle_images[] = {&idle_img1};

static const void *typing_images[] = {&fast_img1, &fast_img2};

static void set_img_src(void *var, int32_t val) {
    LOG_DBG("BONGO: Animation frame callback: %d", val);
    lv_obj_t *img = (lv_obj_t *)var;

    if (anim_state.current_images && anim_state.num_frames > 0) {
        int frame = val % anim_state.num_frames;
        LOG_DBG("BONGO: Setting frame %d of %d", frame, anim_state.num_frames);
        lv_img_set_src(img, anim_state.current_images[frame]);
    }
}

static void update_bongo_cat_anim(struct zmk_widget_bongo_cat *widget, bool is_typing) {
    LOG_DBG("BONGO: Updating animation, is_typing: %d", is_typing);

    if (!widget || !widget->obj) {
        LOG_ERR("BONGO: Widget or object is NULL!");
        return;
    }

    // Only update if state changed
    if (is_typing == anim_state.is_typing) {
        return;
    }

    // Stop any existing animation
    lv_anim_del(widget->obj, set_img_src);

    anim_state.is_typing = is_typing;

    // Set up new animation
    if (!is_typing) {
        LOG_DBG("BONGO: Setting idle animation");
        anim_state.current_images = idle_images;
        anim_state.num_frames = 1;
        lv_img_set_src(widget->obj, idle_images[0]);
    } else {
        LOG_DBG("BONGO: Setting typing animation");
        anim_state.current_images = typing_images;
        anim_state.num_frames = 2;

        lv_anim_init(&widget_anim);
        lv_anim_set_var(&widget_anim, widget->obj);
        lv_anim_set_values(&widget_anim, 0, 1);
        lv_anim_set_time(&widget_anim, 200);
        lv_anim_set_exec_cb(&widget_anim, set_img_src);
        lv_anim_set_repeat_count(&widget_anim, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&widget_anim);
    }
}

static void bongo_cat_update_cb(int wpm) {
    LOG_DBG("BONGO: Update callback triggered, WPM: %d", wpm);
    struct zmk_widget_bongo_cat *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { update_bongo_cat_anim(widget, wpm > 0); }
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

    // Initialize with idle animation
    anim_state.current_images = idle_images;
    anim_state.num_frames = 1;
    anim_state.is_typing = false;
    lv_img_set_src(widget->obj, idle_images[0]);

    sys_slist_append(&widgets, &widget->node);

    LOG_DBG("BONGO: === INITIALIZATION COMPLETE ===");
    return 0;
}

static void bongo_cat_listener_cb(const zmk_event_t *eh) {
    struct zmk_wpm_state_changed *ev = as_zmk_wpm_state_changed(eh);
    if (ev) {
        bongo_cat_update_cb(ev->state);
    }
}

ZMK_LISTENER(widget_bongo_cat, bongo_cat_listener_cb);
ZMK_SUBSCRIPTION(widget_bongo_cat, zmk_wpm_state_changed);

lv_obj_t *zmk_widget_bongo_cat_obj(struct zmk_widget_bongo_cat *widget) { return widget->obj; }
