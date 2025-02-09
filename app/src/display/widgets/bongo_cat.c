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

#if IS_ENABLED(CONFIG_ZMK_WIDGET_BONGO_CAT)
LOG_DBG("=== BONGO CAT MODULE COMPILED IN ===");
#endif

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

static lv_anim_t widget_anim;

static enum anim_state { anim_state_idle, anim_state_typing } current_anim_state = anim_state_idle;

LV_IMG_DECLARE(idle_img1);
LV_IMG_DECLARE(idle_img2);
LV_IMG_DECLARE(idle_img3);
LV_IMG_DECLARE(idle_img4);
LV_IMG_DECLARE(idle_img5);

LV_IMG_DECLARE(fast_img1);
LV_IMG_DECLARE(fast_img2);

static const void *idle_images[] = {
    &idle_img1, &idle_img2, &idle_img3, &idle_img4, &idle_img5,
};

static const void *typing_images[] = {
    &fast_img1,
    &fast_img2,
};

struct bongo_cat_state {
    int wpm;
    enum anim_state anim;
    const void **current_images;
    int current_frame;
};

static void set_img_src(void *var, int val) {
    LOG_DBG("Setting image source: index=%d", val);
    lv_obj_t *img = (lv_obj_t *)var;
    struct bongo_cat_state *state = (struct bongo_cat_state *)lv_obj_get_user_data(img);
    if (state && state->current_images) {
        lv_img_set_src(img, state->current_images[val]);
        state->current_frame = val;
        LOG_DBG("Image source set successfully to frame %d", val);
    }
}

static void update_bongo_cat_anim(struct zmk_widget_bongo_cat *widget,
                                  struct bongo_cat_state state) {
    LOG_DBG("=== Animation Update Start ===");
    LOG_DBG("Current WPM: %d", state.wpm);
    LOG_DBG("Current animation state: %d", current_anim_state);
    LOG_DBG("Target animation state: %d", state.anim);

    struct bongo_cat_state *stored_state = lv_obj_get_user_data(widget->obj);
    if (!stored_state) {
        stored_state = k_malloc(sizeof(struct bongo_cat_state));
        lv_obj_set_user_data(widget->obj, stored_state);
    }
    *stored_state = state;

    if (state.wpm == 0) {
        if (current_anim_state != anim_state_idle) {
            LOG_DBG("Switching to idle animation");
            stored_state->current_images = idle_images;
            lv_anim_init(&widget_anim);
            lv_anim_set_var(&widget_anim, widget->obj);
            lv_anim_set_time(&widget_anim, 1000);
            lv_anim_set_values(&widget_anim, 0, 4);
            lv_anim_set_exec_cb(&widget_anim, set_img_src);
            lv_anim_set_repeat_count(&widget_anim, LV_ANIM_REPEAT_INFINITE);
            lv_anim_start(&widget_anim);
            current_anim_state = anim_state_idle;
            LOG_DBG("Idle animation started");
        }
    } else {
        if (current_anim_state != anim_state_typing) {
            LOG_DBG("Switching to typing animation");
            stored_state->current_images = typing_images;
            lv_anim_init(&widget_anim);
            lv_anim_set_var(&widget_anim, widget->obj);
            lv_anim_set_time(&widget_anim, 100); // Faster animation
            lv_anim_set_values(&widget_anim, 0, 1);
            lv_anim_set_exec_cb(&widget_anim, set_img_src);
            lv_anim_set_repeat_count(&widget_anim, LV_ANIM_REPEAT_INFINITE);
            lv_anim_start(&widget_anim);
            current_anim_state = anim_state_typing;
            LOG_DBG("Typing animation started");
        }
    }
    LOG_DBG("=== Animation Update Complete ===");
}

static struct bongo_cat_state bongo_cat_get_state(const zmk_event_t *eh) {
    LOG_DBG("Getting bongo cat state");
    struct bongo_cat_state state = {
        .wpm = zmk_wpm_get_state(), .current_images = NULL, .current_frame = 0};

    state.anim = (state.wpm == 0) ? anim_state_idle : anim_state_typing;
    LOG_DBG("State determined - WPM: %d, Animation State: %d", state.wpm, state.anim);
    return state;
}

static void bongo_cat_update_cb(struct bongo_cat_state state) {
    LOG_DBG("Update callback triggered");
    LOG_DBG("Updating all bongo cat widgets with WPM: %d", state.wpm);

    struct zmk_widget_bongo_cat *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { update_bongo_cat_anim(widget, state); }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_bongo_cat, struct bongo_cat_state, bongo_cat_update_cb,
                            bongo_cat_get_state)
ZMK_SUBSCRIPTION(widget_bongo_cat, zmk_wpm_state_changed);

int zmk_widget_bongo_cat_init(struct zmk_widget_bongo_cat *widget, lv_obj_t *parent) {
    LOG_ERR("=== BONGO CAT INITIALIZATION START ==="); // Using ERR level temporarily for visibility
    if (widget == NULL || parent == NULL) {
        LOG_ERR("Invalid widget or parent");
        return -1;
    }
    LOG_ERR("Widget and parent valid");

    LOG_ERR("Creating LVGL image object");
    widget->obj = lv_img_create(parent);
    if (widget->obj == NULL) {
        LOG_ERR("CRITICAL: Failed to create LVGL image object");
        return -1;
    }
    LOG_ERR("LVGL image object created at %p", (void *)widget->obj);

    LOG_ERR("Checking image data...");
    if (&idle_img1 == NULL) {
        LOG_ERR("idle_img1 is NULL!");
    } else {
        LOG_ERR("idle_img1 found at %p", (void *)&idle_img1);
    }

    LOG_ERR("Getting initial state");
    struct bongo_cat_state initial_state = bongo_cat_get_state(NULL);
    LOG_ERR("Initial WPM: %d", initial_state.wpm);

    LOG_ERR("Initializing animation");
    update_bongo_cat_anim(widget, initial_state);

    LOG_ERR("Adding widget to list");
    sys_slist_append(&widgets, &widget->node);

    LOG_ERR("Initializing widget listener");
    widget_bongo_cat_init();

    LOG_ERR("=== BONGO CAT INITIALIZATION COMPLETE ===");
    return 0;
}

lv_obj_t *zmk_widget_bongo_cat_obj(struct zmk_widget_bongo_cat *widget) { return widget->obj; }
