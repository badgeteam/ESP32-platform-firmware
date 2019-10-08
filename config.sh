#!/bin/bash
git submodule update --init --recursive
cd firmware
bash mpy_cross.sh
make menuconfig
