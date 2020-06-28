#!/bin/bash
export PATH="$PATH:$(pwd)/xtensa-esp32-elf/bin"

# Load config variables so we know about the currently selected badge
source firmware/sdkconfig

# Read partition information from partition table CSV
OFFSET="$(python3 scripts/print_partition_info.py firmware/$CONFIG_PARTITION_TABLE_CUSTOM_FILENAME locfd offset)"
SIZE="$(python3 scripts/print_partition_info.py firmware/$CONFIG_PARTITION_TABLE_CUSTOM_FILENAME locfd size)"

echo "Size: "$SIZE
echo "Offset: "$OFFSET

# Prepare FAT image
cd firmware/components/mkfatfs/src
make
./mkfatfs -c ../../../../initial_filesystems/$CONFIG_INFO_HARDWARE_FOLDER -s $SIZE ../../../../initial_fs.img -d 2
cd ../../../../

# Flash image to ESP32
python3 esp-idf/components/esptool_py/esptool/esptool.py --port \
  $(bash -c 'ls /dev/tty{USB*,.wchusbserial*,.usbmodem*,.SLAB_USBtoUART,ACM*} 2>/dev/null') \
  write_flash $OFFSET fs.dump #initial_fs.img

# And clean up again
#rm initial_fs.img
