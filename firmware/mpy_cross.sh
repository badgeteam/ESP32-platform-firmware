#!/bin/bash

build_MPyCross() {
    if [ ! -f "components/micropython/mpy-cross/mpy-cross" ]; then
        export CROSS_COMPILE=""
        cd components/mpy_cross_build/mpy-cross
        echo "=================="
        echo "Building mpy-cross"
        make clean > /dev/null 2>&1
        make > /dev/null 2>&1
        if [ $? -eq 0 ]; then
            cp -f mpy-cross ../../micropython/mpy-cross > /dev/null 2>&1
            make clean > /dev/null 2>&1
            echo "OK."
            echo "=================="
        else
            echo "FAILED"
            echo "=================="
            return 1
        fi
        cd ../../../
        if [ ! -f "components/micropython/mpy-cross/mpy-cross" ]; then
            echo "FAILED"
            echo "=================="
            return 1
        fi
    fi
    return 0
}

build_MPyCross
