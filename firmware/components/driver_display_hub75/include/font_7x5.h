//
// Created by Tom on 12/07/2019.
//

#ifndef NEW_ESP32_FIRMWARE_FONT_7X5_H
#define NEW_ESP32_FIRMWARE_FONT_7X5_H

#include <sdkconfig.h>
#include <stdint.h>
#include "stdlib.h"
#include "color.h"

void renderChar_7x5(uint8_t charId, Color color, int *x, int y, int endX, int *skip);
int getCharWidth_7x5(uint8_t charId);

#endif //NEW_ESP32_FIRMWARE_FONT_7X5_H
