//
// Created by Tom on 12/07/2019.
//

#ifndef NEW_ESP32_FIRMWARE_FONT_6X3_H
#define NEW_ESP32_FIRMWARE_FONT_6X3_H

#include <sdkconfig.h>
#include <stdint.h>
#include "stdlib.h"
#include "color.h"

void renderChar_6x3(uint8_t charId, Color color, int *x, int y, int endX, int *skip);
int getCharWidth_6x3(uint8_t charId);

#endif //NEW_ESP32_FIRMWARE_FONT_6X3_H
