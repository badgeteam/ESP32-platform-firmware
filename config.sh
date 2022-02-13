#!/usr/bin/env bash
git submodule update --init --recursive
export PATH="$PATH:$(pwd)/xtensa-esp32-elf/bin"
cd firmware
bash mpy_cross.sh
make menuconfig
