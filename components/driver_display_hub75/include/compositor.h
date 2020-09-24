#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "color.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct renderTask {
    struct renderTask *next;
    void *payload;
    int id;
    int x;
    int y;
    int sizeX;
    int sizeY;
    Color color;
} renderTask_t;

typedef struct animation {
    uint8_t *gif;
    int showFrame;
    int numberFrames;
} animation_t;

typedef struct scrollText {
    char *text;
    int skip;
    int speed;
    bool firstshow;
} scrollText_t;

void compositor_init();

void compositor_clear();

void compositor_setBackground(Color color);
void compositor_setPixel(int x, int y, Color color);

void compositor_addText(char *text, Color color, int x, int y);
void compositor_addScrollText(char *text, Color color, int x, int y, int sizeX);
void compositor_addAnimation(uint8_t *image, int x, int y, int width, int length, int numFrames);
void compositor_addImage(uint8_t *image, int x, int y, int width, int length);

unsigned int compositor_getTextWidth(char *text);
void compositor_setFont(int index);

void composite();
void compositor_setBuffer(Color *framebuffer);
void compositor_enable();
void compositor_disable();
bool compositor_status();
void display_crash();

#ifdef __cplusplus
}
#endif
