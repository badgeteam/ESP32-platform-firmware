#!/bin/bash
export PATH="$PATH:$(pwd)/xtensa-esp32-elf/bin"
python2 esp-idf/components/esptool_py/esptool/esptool.py --port $(bash -c 'ls /dev/tty{USB0,.wchusbserial*,.usbmodem*,.SLAB_USBtoUART,ACMx} 2>/dev/null') erase_flash
