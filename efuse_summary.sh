#!/usr/bin/env bash
python3 esp-idf/components/esptool_py/esptool/espefuse.py --port $(bash -c 'ls /dev/tty{USB*,.wchusbserial*,.usbmodem*,.SLAB_USBtoUART,ACM*} 2>/dev/null') --baud 115200 summary