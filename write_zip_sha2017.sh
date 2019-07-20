#!/bin/bash
source set_env.sh

cd first_boot_files
zip -0 ../files.zip -xi *
cd ..

python2 esp-idf/components/esptool_py/esptool/esptool.py --port /dev/ttyUSB0 write_flash 0x191000 files.zip
