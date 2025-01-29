# Zephyr™ Mechanical Keyboard (ZMK) Firmware

My fork of ZMK with bongo cat working for nice nano v2 with SSD 1306 OLEDs (128x32 pixels). 

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
You can use this zmk config repo as an example: https://github.com/SamIAm2000/zmk-config-lagom-bongo/tree/master.

![IMG_7867](https://github.com/user-attachments/assets/e41f7551-ddc4-4e1a-a272-19fac4849128)
