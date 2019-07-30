#ifndef _DRIVER_FRAMEBUFFER_H_
#define _DRIVER_FRAMEBUFFER_H_
#include "sdkconfig.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_system.h"

#include "driver_framebuffer_font.h"
#include "driver_framebuffer_orientation_internal.h"
#include "driver_framebuffer_dirty.h"
#include "driver_framebuffer_compositor.h"
#include "driver_framebuffer_orientation.h"
#include "driver_framebuffer_drawing.h"
#include "driver_framebuffer_text.h"

//PNG library
#include "mem_reader.h"
#include "file_reader.h"
#include "png_reader.h"

/* Flags */
#define FB_FLAG_FORCE          1
#define FB_FLAG_FULL           2
#define FB_FLAG_LUT_GREYSCALE  4
#define FB_FLAG_LUT_NORMAL     8
#define FB_FLAG_LUT_FAST      16
#define FB_FLAG_LUT_FASTEST   32

esp_err_t driver_framebuffer_init();
/* Initialize the framebuffer driver (called once at system boot from platform.c) */

void driver_framebuffer_flush(uint32_t flags);
/* Flush the framebuffer to the display */

void driver_framebuffer_set_greyscale(bool use);
void driver_framebuffer_setFlags(uint8_t newFlags);

//Size
uint16_t driver_framebuffer_getWidth(void);
uint16_t driver_framebuffer_getHeight(void);

//Drawing
void driver_framebuffer_setPixel(Frame* frame, int16_t x, int16_t y, uint32_t value);
uint32_t driver_framebuffer_getPixel(Frame* frame, int16_t x, int16_t y);
void driver_framebuffer_fill(Frame* frame, uint32_t value);

//Image decoders
esp_err_t driver_framebuffer_png(Frame* frame, int16_t x, int16_t y, lib_reader_read_t reader, void* reader_p);

//Colors
#define COLOR_BLACK 0x000000
#define COLOR_WHITE 0xFFFFFF
#define COLOR_RED   0xFF0000
#define COLOR_GREEN 0x00FF00
#define COLOR_BLUE  0x0000FF

#endif //_DRIVER_FRAMEBUFFER_H_
