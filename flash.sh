#!/usr/bin/env bash
export PATH="$PATH:$(pwd)/xtensa-esp32-elf/bin"
cd firmware
make flash
