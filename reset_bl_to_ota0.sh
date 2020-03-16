#!/bin/bash
export PATH="$PATH:$(pwd)/xtensa-esp32-elf/bin"
python2 esp-idf/components/esptool_py/esptool/esptool.py --port `ls /dev/tty{USB0,.wchusbserial*,.SLAB_USBtoUART,ACMx} 2>/dev/null` erase_region 0xd000 0x2000
