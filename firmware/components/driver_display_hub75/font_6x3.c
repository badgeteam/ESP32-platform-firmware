//
// Created by Tom on 12/07/2019.
//
#include <sdkconfig.h>
#include "include/color.h"
#include "include/font_6x3.h"
#include "include/compositor.h"
#include "esp_log.h"

#ifdef CONFIG_DRIVER_HUB75_ENABLE

void renderChar_6x3(uint8_t charId, Color color, int *x, int y, int endX, int *skip) {
    character toDraw = font_6x3[charId];

    for (int col = 0; col < toDraw.width; col++) {
        if (*skip != 0) {
            (*skip)--;
            continue;
        }

        for (int row = 0; row < 6; row++) {
            int8_t targetRow = y + row;
            int8_t targetCol = *x + col;

            if (targetCol >= CONFIG_HUB75_WIDTH || targetRow >= CONFIG_HUB75_HEIGHT || targetCol < 0 || targetRow < 0) {
                continue;
            }

            uint8_t pixel = (toDraw.rows[row] >> (toDraw.width - 1 - col)) & 0b01;

            if (pixel) {
                compositor_setPixel(*x, y + row, color);
            }
        }

        (*x)++;
        if (endX > 0 && *x >= endX) {
            return;
        }
    }
}

int getCharWidth_6x3(uint8_t charId) {
    character c = font_6x3[charId];
    return (int) c.width;
}

#endif
