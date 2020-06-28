#!/bin/bash
export PATH="$PATH:$(pwd)/xtensa-esp32-elf/bin"

# Load config variables so we know about the currently selected badge
source firmware/sdkconfig

# Read partition information from partition table CSV
OFFSET="$(python3 scripts/print_partition_info.py firmware/$CONFIG_PARTITION_TABLE_CUSTOM_FILENAME ota_1 offset)"
SIZE="$(python3 scripts/print_partition_info.py firmware/$CONFIG_PARTITION_TABLE_CUSTOM_FILENAME ota_1 size)"

# Prepare zip file
cd initial_filesystems/$CONFIG_INFO_HARDWARE_FOLDER
zip -r initial_fs.zip . -x "*.DS_Store"
cd ../..

# TODO: check if zip fits in ota partition

# Flash zip file to the OTA partition that it's not booting from
python3 esp-idf/components/esptool_py/esptool/esptool.py --port \
  $(bash -c 'ls /dev/tty{USB*,.wchusbserial*,.usbmodem*,.SLAB_USBtoUART,ACM*} 2>/dev/null') \
  write_flash $OFFSET initial_filesystems/$CONFIG_INFO_HARDWARE_FOLDER/initial_fs.zip

# And clean up again
rm initial_filesystems/$CONFIG_INFO_HARDWARE_FOLDER/initial_fs.zip
