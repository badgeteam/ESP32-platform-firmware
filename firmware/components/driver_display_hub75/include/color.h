//
// Created by Tom on 12/07/2019.
//

#ifndef NEW_ESP32_FIRMWARE_COLOR_H
#define NEW_ESP32_FIRMWARE_COLOR_H

#include <stdint.h>

typedef union{
    uint32_t value;
    uint8_t RGB[4];
} Color;

#endif //NEW_ESP32_FIRMWARE_COLOR_H
