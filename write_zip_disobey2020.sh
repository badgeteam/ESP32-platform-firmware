#!/bin/bash
export PATH="$PATH:$(pwd)/xtensa-esp32-elf/bin"
python2 esp-idf/components/esptool_py/esptool/esptool.py --port /dev/ttyUSB0 write_flash 0x191000 initial_files_disobey2020.zip
