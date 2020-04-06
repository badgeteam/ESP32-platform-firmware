#!/bin/bash
python2 esp-idf/components/esptool_py/esptool/espefuse.py --port $(bash -c 'ls /dev/tty{USB0,.wchusbserial*,.usbmodem*,.SLAB_USBtoUART,ACMx} 2>/dev/null') --baud 115200 summary