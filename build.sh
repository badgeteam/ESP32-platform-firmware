#!/bin/bash
git submodule update --init --recursive || exit 1
cd firmware
export PATH="$PATH:`pwd`/xtensa-esp32-elf/bin"
bash mpy_cross.sh || exit 1
rm build -rf
make clean
make -j8
