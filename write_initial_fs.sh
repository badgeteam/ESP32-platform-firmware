#!/bin/bash
export PATH="$PATH:$(pwd)/xtensa-esp32-elf/bin"

# Load config variables so we know about the currently selected badge
source firmware/sdkconfig

# Read partition information from partition table CSV
OFFSET="$(python scripts/print_partition_info.py firmware/$CONFIG_PARTITION_TABLE_CUSTOM_FILENAME locfd offset)"
SIZE="$(python scripts/print_partition_info.py firmware/$CONFIG_PARTITION_TABLE_CUSTOM_FILENAME locfd size)"

# Prepare FAT image
cd firmware/components/mkfatfs/src
make
./mkfatfs -c ../../../../initial_filesystems/$CONFIG_INFO_HARDWARE_FOLDER -s $SIZE ../../../../initial_fs.img
cd ../../../../

# Flash image to ESP32
#python2 esp-idf/components/esptool_py/esptool/esptool.py --port \
#  `ls /dev/tty{USB0,.wchusbserial*,.SLAB_USBtoUART,ACMx} 2>/dev/null` \
#  write_flash $OFFSET initial_fs.img

# And clean up again
#rm initial_fs.img