#ifndef _DRIVER_FRAMEBUFFER_TEXT_H_
#define _DRIVER_FRAMEBUFFER_TEXT_H_

#include <stdint.h>
#include <stdbool.h>

/* Fonts */
extern const GFXfont fairlight8pt7b;
extern const GFXfont fairlight12pt7b;
extern const GFXfont freesans6pt7b;
extern const GFXfont freesans8pt7b;
extern const GFXfont freesans9pt7b;
extern const GFXfont freesans12pt7b;
extern const GFXfont freesansbold9pt7b;
extern const GFXfont freesansbold12pt7b;
extern const GFXfont freesansmono8pt7b;
extern const GFXfont freesansmono9pt7b;
extern const GFXfont freesansmono12pt7b;
extern const GFXfont org_018pt7b;
extern const GFXfont dejavusans20pt7b;
extern const GFXfont permanentmarker22pt7b;
extern const GFXfont permanentmarker36pt7b;
extern const GFXfont robotoblack22pt7b;
extern const GFXfont robotoblackitalic24pt7b;
extern const GFXfont roboto12pt7b;
extern const GFXfont roboto18pt7b;
extern const GFXfont roboto22pt7b;
extern const GFXfont pixelade9pt7b;
extern const GFXfont pixelade13pt7b;
extern const GFXfont weather42pt8b;

void driver_framebuffer_setFont(const GFXfont *font);
bool driver_framebuffer_selectFont(const char* fontName);
void driver_framebuffer_setCursor(int16_t x, int16_t y);
void driver_framebuffer_getCursor(int16_t* x, int16_t* y);
void driver_framebuffer_setTextScale(uint8_t w, uint8_t h);
void driver_framebuffer_getTextScale(uint8_t* w, uint8_t* h);
void driver_framebuffer_setTextColor(uint32_t value);
uint32_t driver_framebuffer_getTextColor();
void print_char(int16_t x0, int16_t y0, unsigned char c, uint8_t xScale, uint8_t yScale, uint32_t color);
void driver_framebuffer_write(uint8_t c);
uint8_t driver_framebuffer_get_font_height();
uint16_t driver_framebuffer_get_char_width(uint8_t c);
void driver_framebuffer_print(const char* str);
uint16_t driver_framebuffer_get_string_width(const char* str);
uint16_t driver_framebuffer_get_string_height(const char* str);
void driver_framebuffer_print_len(const char* str, int16_t len);

#endif
