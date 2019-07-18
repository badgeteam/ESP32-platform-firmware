#ifndef _DRIVER_FRAMEBUFFER_H_
#define _DRIVER_FRAMEBUFFER_H_
#include "sdkconfig.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "driver_framebuffer_font.h"
#include "esp_system.h"

/* Fonts */
extern const GFXfont fairlight8pt7b;
extern const GFXfont fairlight9pt7b;
extern const GFXfont fairlight12pt7b;
extern const GFXfont freesans8pt7b;
extern const GFXfont freesans9pt7b;
extern const GFXfont freesans12pt7b;
extern const GFXfont freesansbold8pt7b;
extern const GFXfont freesansbold9pt7b;
extern const GFXfont freesansbold12pt7b;
extern const GFXfont freesansmono8pt7b;
extern const GFXfont freesansmono9pt7b;
extern const GFXfont freesansmono12pt7b;
extern const GFXfont org_018pt7b;
extern const GFXfont org_019pt7b;
extern const GFXfont org_0112pt7b;

//SHA2017
extern const GFXfont dejavusans20pt7b;
extern const GFXfont permanentmarker22pt7b;
extern const GFXfont permanentmarker36pt7b;
extern const GFXfont robotoblack22pt7b;
extern const GFXfont robotoblackitalic24pt7b;
extern const GFXfont roboto12pt7b;
extern const GFXfont roboto18pt7b;
extern const GFXfont roboto22pt7b;
extern const GFXfont pixelade13pt7b;
extern const GFXfont weather42pt8b;


#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
bool currentFb;
uint8_t* framebuffer1, framebuffer2;
#else
uint8_t* framebuffer;
#endif

uint16_t driver_framebuffer_get_orientation();
void driver_framebuffer_set_orientation(uint16_t angle);

esp_err_t driver_framebuffer_init();
void driver_framebuffer_setCursor(int16_t x, int16_t y);
void driver_framebuffer_getCursor(int16_t* x, int16_t* y);
void driver_framebuffer_write(uint8_t c);
void driver_framebuffer_print(const char* str);
void driver_framebuffer_print_len(const char* str, int16_t len);
void driver_framebuffer_setScale(int16_t x, int16_t y);
void driver_framebuffer_setFont(const GFXfont *font);
void driver_framebuffer_setFlags(uint8_t newFlags);
void driver_framebuffer_flush();

void driver_framebuffer_get_dirty(int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1);
bool driver_framebuffer_is_dirty();
void driver_framebuffer_set_greyscale(bool use);

void driver_framebuffer_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color);
void driver_framebuffer_rect(int16_t x, int16_t y, uint16_t w, uint16_t h, bool fill, uint32_t color);
void driver_framebuffer_circle(int16_t x0, int16_t y0, uint16_t r, uint16_t a0, uint16_t a1, bool fill, uint32_t color);
void driver_framebuffer_char(int16_t x0, int16_t y0, unsigned char c, uint8_t xScale, uint8_t yScale, uint32_t color);
void driver_framebuffer_setTextColor(uint32_t color);

bool driver_framebuffer_selectFont(const char* fontName);

esp_err_t driver_framebuffer_png(int16_t x, int16_t y, const uint8_t* png_data, size_t len);

void driver_framebuffer_fill(uint32_t value);
void driver_framebuffer_pixel(int16_t x, int16_t y, uint32_t value);
uint32_t driver_framebuffer_getPixel(int16_t x, int16_t y);

int16_t driver_framebuffer_getWidth(void);
int16_t driver_framebuffer_getHeight(void);

#define COLOR_BLACK 0x000000
#define COLOR_WHITE 0xFFFFFF
#define COLOR_RED   0xFF0000
#define COLOR_GREEN 0x00FF00
#define COLOR_BLUE  0x0000FF

#endif //_DRIVER_FRAMEBUFFER_H_
