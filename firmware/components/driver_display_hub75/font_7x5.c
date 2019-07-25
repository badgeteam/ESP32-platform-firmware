//
// Created by Tom on 12/07/2019.
//

#include "include/font_7x5.h"
#include "include/compositor.h"

#ifdef CONFIG_DRIVER_HUB75_ENABLE

void renderCharCol(uint8_t ch, Color color, int x, int y) {
    for(int py = y; py<y+7; py++) {
        if(py >= 0 && py < CONFIG_HUB75_HEIGHT && x >= 0 && x < CONFIG_HUB75_WIDTH) {
            if((ch & (1<<(py-y))) != 0) compositor_setPixel(x, py, color);
        }
    }
}

void renderChar_7x5(uint8_t charId, Color color, int *x, int y, int endX, int *skip) {
    for(int i = 0; i<5; i++) {
        if(*skip == 0) {
            uint8_t cs = font_7x5[charId*5+i];
            if(endX > 0 && *x >= endX) return;
            renderCharCol(cs, color, *x, y);
            (*x)++;
        } else {
            (*skip)--;
        }
    }
}

int getCharWidth_7x5(uint8_t charId) {
    return 5;
}

#endif
