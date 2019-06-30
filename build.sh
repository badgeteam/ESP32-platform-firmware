#!/bin/bash
cd firmware
export IDF_PATH="../esp-idf"
#bash submodules.sh
#source set_env.sh
bash mpy_cross.sh
rm build -rf
make clean
make -j8
