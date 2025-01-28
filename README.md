# Zephyr™ Mechanical Keyboard (ZMK) Firmware

My fork of ZMK with bongo cat working for nice nano v2 with SSD 1306 OLEDs (128x32 pixels). 

It works for using ZMK wired, haven't tested wireless but I imagine it wll drain the battery pretty fast.

To add bongo cat on your keyboard, use this fork of ZMK and the `bongo-cat` branch in the west.yml of your ZMK build. 

Add these lines to your `boardname.conf` file:
```
CONFIG_ZMK_DISPLAY=y
CONFIG_ZMK_DISPLAY_WORK_QUEUE_DEDICATED=y
CONFIG_ZMK_WPM=y
CONFIG_ZMK_WIDGET_WPM_STATUS=y
CONFIG_ZMK_WIDGET_LAYER_STATUS=n
CONFIG_ZMK_WIDGET_BATTERY_STATUS=n
CONFIG_ZMK_WIDGET_OUTPUT_STATUS=n
```
![IMG_7867](https://github.com/user-attachments/assets/e41f7551-ddc4-4e1a-a272-19fac4849128)
