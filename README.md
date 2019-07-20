# New badge.team ESP32 Firmware

This repository contains the reworked firmware platform for generic ESP32-based hardware devices.

badge.team firmware has been used by many event badges, such as:
* SHA2017
* HackerHotel 2019
* Disobey 2019
* CampZone 2019
 
<!--# Resources

* [Project documentation](https://wiki.badge.team)
* [Documentation](https://wiki.badge.team/Firmware)
* [Firmware](https://github.com/badgeteam/ESP32-Firmware)
* [Changelog](CHANGELOG.md)


[![Codacy Badge](https://api.codacy.com/project/badge/Grade/a61bf7ca8c6040e78382af2741a67d04)](https://www.codacy.com/app/Badgeteam/ESP32-Firmware?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=badgeteam/ESP32-Firmware&amp;utm_campaign=Badge.team)
[![Build Status](https://travis-ci.org/badgeteam/ESP32-Firmware.svg?branch=master)](https://travis-ci.org/badgeteam/ESP32-Firmware)
-->

## Debian prerequisites

```
sudo apt-get install libncurses5-dev flex bison gperf python-serial libffi-dev libsdl2-dev libmbedtls-dev perl
```

## Preparing your setup

First, make sure you pull the submodules in the project:

```
git submodule update --init --recursive
```

Next, copy the xtensa build toolchain for your OS (currently supporting Linux and Mac OS) from /toolchains/, and unpack and save it as /xtensa-esp32-elf/ in the project root folder.
# Build instructions
To build and flash the basic generic firmware:
```
./build.sh
./flash.sh
 ```
 
Make sure you have downloaded the appropriate driver for the USB UART chip on your device. Below are some from popular badges.

* SHA2017 / HackerHotel 2019: [CP2102 driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
* CampZone 2019: [CH340C driver](https://learn.sparkfun.com/tutorials/sparkfun-serial-basic-ch340c-hookup-guide/drivers-if-you-need-them)

# Interacting via serial
By default, the badge.team firmware activates a simple python shell or serial menu after booting. You can interact with it by running:
```
./monitor.sh
```

# Building for a specific existing badge
Copy the relevant config file from `/firmware/configs/` to `/firmware/sdkconfig`, build, flash, and enjoy!

# Creating new hardware
You can use badge.team firmware for your own-designed badge or device. The workflow for this is described below.

 * Run `./config.sh`, configure the `Firmware & device configuration` submenu (for the folder option in the `Hardware description` section, think of any name. We will use it later.), and set up the drivers that your hardware will use in the `Components` submenu.
 * If you need to write drivers of your own, take a look at the [driver documentation](DRIVERS.md).
 * Create a directory with the folder name you configured in the first step in `/firmware/python_modules`, and copy `/firmware/python_modules/generic/*` into it.
 * In this directory, you can write Python code that will be built into the firmware image. The files `_boot.py` and `boot.py` are executed after each other on boot, and from there you can launch your own things.
 * Build and flash, and you're done!
 
<!--

# MicroPython

```
import badge
badge.eink_init()
badge.display_picture(0,-1)
import ugfx
ugfx.init()
ugfx.demo("HACKING")
ugfx.clear(ugfx.BLACK)
ugfx.thickline(1,1,100,100,ugfx.WHITE,10,5)
ugfx.box(30,30,50,50,ugfx.WHITE)
ugfx.string(150,25,"STILL","Roboto_BlackItalic24",ugfx.WHITE)
ugfx.string(130,50,"Hacking","PermanentMarker22",ugfx.WHITE)
len = ugfx.get_string_width("Hacking","PermanentMarker22")
ugfx.line(130, 72, 144 + len, 72, ugfx.WHITE)
ugfx.line(140 + len, 52, 140 + len, 70, ugfx.WHITE)
ugfx.string(140,75,"Anyway","Roboto_BlackItalic24",ugfx.WHITE)
ugfx.flush()
```
More info on the [MicroPython badge features](https://wiki.badge.team/MicroPython) -->

Copyright (C) 2017-2019 Badge team.
Using [Espressif Audio Development Framework](https://github.com/espressif/esp-adf) Copyright (C) 2018 Espressif Systems.
Based on template application for [Espressif IoT Development Framework (ESP-IDF)](https://github.com/espressif/esp-idf).
Copyright (C) 2016 Espressif Systems, licensed under the Apache License 2.0 as described in the file LICENSE.
