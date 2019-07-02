#!/bin/bash
source set_env.sh
export HOST_PLATFORM=${machine}
git submodule update --init --recursive
cd firmware
bash mpy_cross.sh
rm build -rf
make clean
make -j8
