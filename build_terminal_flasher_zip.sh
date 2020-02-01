#!/bin/bash
mkdir terminal_flasher/bootloader
cp firmware/build/disobey2020_16MB.bin firmware/build/ota_data_initial.bin firmware/build/firmware.bin initial_files_disobey2020.zip terminal_flasher/
cp firmware/build/bootloader/bootloader.bin terminal_flasher/bootloader/
zip -r terminal_flasher.zip terminal_flasher

