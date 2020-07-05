#!/bin/bash
export PATH="$PATH:$(pwd)/xtensa-esp32-elf/bin"
./reset_bl_to_ota0.sh
cd firmware
make flash
