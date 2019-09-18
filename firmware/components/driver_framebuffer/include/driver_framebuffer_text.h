#ifndef _DRIVER_FRAMEBUFFER_TEXT_H_
#define _DRIVER_FRAMEBUFFER_TEXT_H_

#include <stdint.h>
#include <stdbool.h>

/* Fonts */

extern const GFXfont fairlight8pt7b;
extern const GFXfont fairlight12pt7b;
extern const GFXfont org_018pt7b;
extern const GFXfont dejavusans20pt7b;
extern const GFXfont permanentmarker22pt7b;
extern const GFXfont permanentmarker36pt7b;
extern const GFXfont robotoblack22pt7b;
extern const GFXfont robotoblackitalic24pt7b;
extern const GFXfont roboto12pt7b;
extern const GFXfont roboto18pt7b;
extern const GFXfont roboto22pt7b;
extern const GFXfont weather42pt8b;
extern const GFXfont ipane7x5;

/* Functions */

const GFXfont* driver_framebuffer_findFontByName(const char* fontName);
uint16_t driver_framebuffer_print(Window* window, const char* str, int16_t x0, int16_t y0, uint8_t xScale, uint8_t yScale, uint32_t color, const GFXfont *font);
uint16_t driver_framebuffer_print_len(Window* window, const char* str, int16_t len, int16_t x0, int16_t y0, uint8_t xScale, uint8_t yScale, uint32_t color, const GFXfont *font);
uint16_t driver_framebuffer_get_string_width(const char* str, const GFXfont *font);
uint16_t driver_framebuffer_get_string_height(const char* str, const GFXfont *font);

#endif
