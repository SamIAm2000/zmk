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

enum anim_state { anim_state_none, anim_state_idle, anim_state_slow, anim_state_fast };

struct bongo_cat_state {
    int wpm;
    enum anim_state anim;
};

LV_IMG_DECLARE(idle_img1);
LV_IMG_DECLARE(idle_img2);
LV_IMG_DECLARE(idle_img3);
LV_IMG_DECLARE(idle_img4);
LV_IMG_DECLARE(idle_img5);

LV_IMG_DECLARE(slow_img);

LV_IMG_DECLARE(fast_img1);
LV_IMG_DECLARE(fast_img2);

const void *idle_images[] = {&idle_img1, &idle_img2, &idle_img3, &idle_img4, &idle_img5};
const void *fast_images[] = {&fast_img1, &fast_img2};

void set_img_src(void *var, int val) {
    lv_obj_t *img = (lv_obj_t *)var;
    lv_img_set_src(img, val);
}

static void update_bongo_cat_anim(struct zmk_widget_bongo_cat *widget,
                                  struct bongo_cat_state state) {
    LOG_DBG("Updating animation state: WPM=%d, AnimState=%d", state.wpm, state.anim);

    if (state.wpm < CONFIG_ZMK_WIDGET_BONGO_CAT_IDLE_LIMIT) {
        if (state.anim != anim_state_idle) {
            lv_anim_init(&widget->anim);
            lv_anim_set_var(&widget->anim, widget->obj);
            lv_anim_set_time(&widget->anim, 1000);
            lv_anim_set_values(&widget->anim, 0, 4);
            lv_anim_set_exec_cb(&widget->anim, set_img_src);
            lv_anim_set_repeat_count(&widget->anim, 10);
            lv_anim_set_repeat_delay(&widget->anim, 100);
            lv_anim_start(&widget->anim);
        }
    } else if (state.wpm < CONFIG_ZMK_WIDGET_BONGO_CAT_SLOW_LIMIT) {
        if (state.anim != anim_state_slow) {
            lv_anim_del(widget->obj, set_img_src);
            lv_img_set_src(widget->obj, &slow_img);
        }
    } else {
        if (state.anim != anim_state_fast) {
            lv_anim_init(&widget->anim);
            lv_anim_set_time(&widget->anim, 200);
            lv_anim_set_repeat_delay(&widget->anim, 200);
            lv_anim_set_var(&widget->anim, widget->obj);
            lv_anim_set_values(&widget->anim, 0, 1);
            lv_anim_set_exec_cb(&widget->anim, set_img_src);
            lv_anim_set_repeat_count(&widget->anim, LV_ANIM_REPEAT_INFINITE);
            lv_anim_start(&widget->anim);
        }
    }
}

static struct bongo_cat_state bongo_cat_get_state(const zmk_event_t *eh) {
    struct bongo_cat_state state = {};
    state.wpm = zmk_wpm_get_state();

    if (state.wpm < CONFIG_ZMK_WIDGET_BONGO_CAT_IDLE_LIMIT) {
        state.anim = anim_state_idle;
    } else if (state.wpm < CONFIG_ZMK_WIDGET_BONGO_CAT_SLOW_LIMIT) {
        state.anim = anim_state_slow;
    } else {
        state.anim = anim_state_fast;
    }

    return state;
}

static void bongo_cat_update_cb(struct bongo_cat_state state) {
    struct zmk_widget_bongo_cat *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { update_bongo_cat_anim(widget, state); }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_bongo_cat, struct bongo_cat_state, bongo_cat_update_cb,
                            bongo_cat_get_state)
ZMK_SUBSCRIPTION(widget_bongo_cat, zmk_wpm_state_changed);

int zmk_widget_bongo_cat_init(struct zmk_widget_bongo_cat *widget, lv_obj_t *parent) {
    widget->obj = lv_img_create(parent);

    // Initialize with the default state
    struct bongo_cat_state initial_state = bongo_cat_get_state(NULL);
    update_bongo_cat_anim(widget, initial_state);

    sys_slist_append(&widgets, &widget->node);
    widget_bongo_cat_init();
    return 0;
}

lv_obj_t *zmk_widget_bongo_cat_obj(struct zmk_widget_bongo_cat *widget) { return widget->obj; }
