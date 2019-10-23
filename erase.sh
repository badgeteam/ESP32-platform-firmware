#!/bin/bash
python2 esp-idf/components/esptool_py/esptool/esptool.py --port `ls /dev/tty{USB0,.wchusbserial*,.SLAB_USBtoUART,ACMx} 2>/dev/null` erase_flash
