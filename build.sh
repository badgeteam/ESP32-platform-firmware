#!/bin/bash
source set_env.sh
git submodule update --init --recursive
bash constants.sh
cd firmware
bash mpy_cross.sh
rm build -rf
make clean
make -j128
