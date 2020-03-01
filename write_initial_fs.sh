#!/bin/bash
export PATH="$PATH:$(pwd)/xtensa-esp32-elf/bin"

# Load config variables so we know about the currently selected badge
source firmware/sdkconfig

cd initial_filesystems/$CONFIG_INFO_HARDWARE_FOLDER
zip -0 ../../files.zip -xi *
cd ../..

python2 esp-idf/components/esptool_py/esptool/esptool.py --port /dev/ttyUSB0 write_flash 0x191000 files.zip
