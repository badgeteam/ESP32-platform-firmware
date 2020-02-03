#!/bin/bash
python3 terminalflash.py /dev/ttyUSB0 /dev/ttyUSB1 /dev/ttyUSB2 /dev/ttyUSB3 ./ ./esptool.py 

## To run without nice GUI: esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 2000000 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0xd000 ota_data_initial.bin 0x1000 bootloader/bootloader.bin 0x8000 disobey2020_16MB.bin 0x10000 firmware.bin 0x1e1000 initial_files_disobey2020.zip
