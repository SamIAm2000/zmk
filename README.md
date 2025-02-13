# Zephyr™ Mechanical Keyboard (ZMK) Firmware

My fork of ZMK with bongo cat working for nice nano v2 with SSD 1306 OLEDs (128x32 pixels). 
---
![IMG_8511](https://github.com/user-attachments/assets/d19f3220-0da9-4c71-b527-1f0c36d0331b)

There are two versions of bongo cat implemented here.

### First Version:
Based on [this implementation of the bongo cat widget by Pete Johansson back in 2022](https://github.com/zmkfirmware/zmk/commit/82ab3c81ae778a3eeacefbb6753d162a7a431595), I just merged his branch into zmk and fixed some deprecated lvgl library errors due to it being a really old branch. This version of bongo cat has animations based on words per minutes (WPM) values - 0-30 is the idle animation, 30-60 is slow, then above 60 is where the cat start bongoing. This means that when you stop typing on your keyboard, the cat will continue to bongo for a bit until WPM reaches 0 (since WPM is a moving average over time). 

A slight problem with this implementation is that sometimes when you're typing really fast, the keyboard inputs lag and might skip key presses because it's single threaded and sending the keypresses gets blocked by the OLED refresh rate. This usually isn't a problem and happens quite sparingly.


https://github.com/user-attachments/assets/3fe8ed61-8ec7-426a-a556-6b7a82deea29


### Second Version (IMO the better version)
This one has two optimizations: It supports a dedicated work queue that separates the main thread and the thread for UI updates, which solves the problem in the first version where key presses were blocking on the OLED screen refresh rate. It also has the bongo cat tapping per key press, and after you stop typing, it goes back to the idle animation. 

https://github.com/user-attachments/assets/ec888576-a20d-4ffb-9358-f41a8e759a0c


## Steps to add both versions
### Version 1
To add this bongo cat on your keyboard, use this fork of ZMK and the `bongo-cat` branch in the west.yml of your ZMK build. 

You can use this zmk config repo as an example: https://github.com/SamIAm2000/zmk-config-lagom-bongo/tree/master.

Add these lines to your `[boardname].conf` file:
```
CONFIG_ZMK_DISPLAY=y
CONFIG_ZMK_WPM=y
CONFIG_ZMK_WIDGET_WPM_STATUS=y
CONFIG_ZMK_WIDGET_LAYER_STATUS=n
CONFIG_ZMK_WIDGET_BATTERY_STATUS=n
CONFIG_ZMK_WIDGET_OUTPUT_STATUS=n
```

Change your west.yaml file to look something like this:
```
manifest:
  remotes:
    - name: SamIAm2000
      url-base: https://github.com/SamIAm2000
  projects:
    - name: zmk
      remote: SamIAm2000
      revision: bongo-cat
      import: app/west.yml
  self:
    path: config
```

### Version 2
To add this bongo cat on your keyboard, use this fork of ZMK and the `bongo-cat-dedicated-work-queue` branch in the west.yml of your ZMK build. 

You can use this ZMK config repo as an example: https://github.com/SamIAm2000/zmk-config-lagom-bongo/tree/bongo-with-dedicated-work-queue.

Add these lines to your `[boardname].conf` file:
```
CONFIG_ZMK_DISPLAY=y
CONFIG_ZMK_DISPLAY_WORK_QUEUE_DEDICATED=y
CONFIG_ZMK_DISPLAY_DEDICATED_THREAD_STACK_SIZE=2048
CONFIG_ZMK_DISPLAY_DEDICATED_THREAD_PRIORITY=1

CONFIG_ZMK_WPM=y
CONFIG_ZMK_WIDGET_WPM_STATUS=y
CONFIG_ZMK_WIDGET_BONGO_CAT=y
CONFIG_ZMK_WIDGET_LAYER_STATUS=n
CONFIG_ZMK_WIDGET_BATTERY_STATUS=n
CONFIG_ZMK_WIDGET_OUTPUT_STATUS=n
```

Change your west.yml file to this:
```
manifest:
  remotes:
    - name: SamIAm2000
      url-base: https://github.com/SamIAm2000
  projects:
    - name: zmk
      remote: SamIAm2000
      revision: bongo-cat-dedicated-work-queue
      import: app/west.yml
  self:
    path: config
```

Your config/boards/shields/[boardname]/Kconfig.defconfig file should look similar to this, but with your keyboard name. You can reference this [file](https://github.com/SamIAm2000/zmk-config-lagom-bongo/blob/bongo-with-dedicated-work-queue/config/boards/shields/radiance/Kconfig.defconfig):
```
if SHIELD_[YOUR_KEYBOARD_NAME]

config ZMK_KEYBOARD_NAME
    default "[your_keyboard_name]"

if ZMK_DISPLAY

config I2C
    default y

config SSD1306
    default y

# Add these lines for Bongo Cat
config LV_USE_ANIMATION
    bool
    default y

config ZMK_WIDGET_BONGO_CAT
    bool
    default y
    select LV_USE_IMG
    select LV_USE_ANIMATION
    
endif # ZMK_DISPLAY

if LVGL

config LV_Z_VDB_SIZE
    default 64

config LV_DPI_DEF
    default 148

config LV_Z_BITS_PER_PIXEL
    default 1

choice LV_COLOR_DEPTH
    default LV_COLOR_DEPTH_1
endchoice

config LV_USE_LABEL
    default y

endif # LVGL

endif # SHIELD_RADIANCE
```


