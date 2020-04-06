#!/bin/bash
export PATH="$PATH:$(pwd)/xtensa-esp32-elf/bin"

# Load config variables so we know about the currently selected badge
source firmware/sdkconfig

# Read partition information from partition table CSV
OFFSET="$(python scripts/print_partition_info.py firmware/$CONFIG_PARTITION_TABLE_CUSTOM_FILENAME otadata offset)"
SIZE="$(python scripts/print_partition_info.py firmware/$CONFIG_PARTITION_TABLE_CUSTOM_FILENAME otadata size)"

# Erase otadata partition
python2 esp-idf/components/esptool_py/esptool/esptool.py --port \
  `ls /dev/tty{USB0,.wchusbserial*,.usbmodem*,.SLAB_USBtoUART,ACMx} 2>/dev/null` \
  erase_region $OFFSET $SIZE
