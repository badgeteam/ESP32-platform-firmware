#!/bin/bash
source set_env.sh
python2 esp-idf/components/esptool_py/esptool/esptool.py --port /dev/ttyUSB0 erase_flash
