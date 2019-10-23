#!/bin/bash
git submodule update --init --recursive
cd firmware
export PATH="$PATH:$(pwd)/xtensa-esp32-elf/bin"
bash mpy_cross.sh
make -j8
