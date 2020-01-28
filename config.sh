#!/bin/bash
export PATH="$PATH:$(pwd)/xtensa-esp32-elf/bin"
git submodule update --init --recursive
cd firmware
bash mpy_cross.sh
make menuconfig
