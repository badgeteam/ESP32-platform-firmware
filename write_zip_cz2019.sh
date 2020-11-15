#!/bin/bash

cd initial_fs
zip -r ../files.zip ./*
cd ..

python2 esp-idf/components/esptool_py/esptool/esptool.py --baud 2000000 --port `ls /dev/tty{USB0,.wchusbserial*,.SLAB_USBtoUART,ACMx} 2>/dev/null` write_flash 0x1e1000 files.zip
rm files.zip