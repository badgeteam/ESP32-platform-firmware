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
sudo apt-get install make unzip git libncurses5-dev flex bison gperf python-serial libffi-dev libsdl2-dev libmbedtls-dev perl
```

## Preparing your setup

First, make sure you pull the submodules in the project:

```
git submodule update --init --recursive
```

Next, copy the xtensa build toolchain for your OS (currently supporting Linux and Mac OS) from /toolchains/, and unpack and save it as /xtensa-esp32-elf/ in the project root folder:

```
unzip -p toolchain/xtensa-esp32-elf-linux64.zip xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar | tar xv
```

# Building for a specific existing badge
Copy the relevant config file from `/firmware/configs/<badge>_defconfig` to `/firmware/sdkconfig`:

```
cp firmware/configs/sha2017_defconfig firmware/sdkconfig
```

# Build instructions
Set the path to esp32-toolchain (you have to repeat that on every new terminal) (usually it is located in your ESP32-platform-firmware directory):
```
export PATH="$PATH:/path/to/my/toolchain/xtensa-esp32-elf/bin"
```

To build and flash the basic generic firmware:
```
./build.sh
./flash.sh
```

Make sure you have downloaded the appropriate driver for the USB UART chip on your device. Below are some from popular badges.

* SHA2017 / HackerHotel 2019: [CP2102 driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
* CampZone 2019: [CH340C driver](https://learn.sparkfun.com/tutorials/sparkfun-serial-basic-ch340c-hookup-guide/drivers-if-you-need-them)

If you have this issue with flashing:
`serial.serialutil.SerialException: [Errno 2] could not open port : [Errno 2] No such file or directory: ''`
Then you need to copy the `python2` command that `./build.sh` suggests, and make sure the `--port ` argument has the right value.

# Interacting via serial
By default, the badge.team firmware activates a simple python shell or serial menu after booting. You can interact with it by running:
```
./monitor.sh
```

# Creating new hardware
You can use badge.team firmware for your own-designed badge or device. The workflow for this is described below.

 * Run `./config.sh`, configure the `Firmware & device configuration` submenu (for the folder option in the `Hardware description` section, think of any name. We will use it later.), and set up the drivers that your hardware will use in the `Components` submenu.
 * If you need to write drivers of your own, take a look at the [driver documentation](DRIVERS.md).
 * Create a directory with the folder name you configured in the first step in `/firmware/python_modules`, and copy `/firmware/python_modules/generic/*` into it.
 * In this directory, you can write Python code that will be built into the firmware image. The files `_boot.py` and `boot.py` are executed after each other on boot, and from there you can launch your own things.
 * Build and flash, and you're done!

# License and information

Copyright (C) 2017-2019 BADGE.TEAM

Uses the [Micropython port for ESP32 by Loboris](https://github.com/loboris/MicroPython_ESP32_psRAM_LoBo)

Uses ESP-IDF by Espressif
