/*
 * BADGE.TEAM framebuffer driver
 * Uses parts of the Adafruit GFX Arduino libray
 */

/*
This is the core graphics library for all our displays, providing a common
set of graphics primitives (points, lines, circles, etc.).  It needs to be
paired with a hardware-specific library for each display device we carry
(to handle the lower-level functions).

Adafruit invests time and resources providing this open source code, please
support Adafruit & open-source hardware by purchasing products from Adafruit!

Copyright (c) 2013 Adafruit Industries.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
 */

#include "include/driver_framebuffer_internal.h"

#include "esp_heap_caps.h"
#include "esp_log.h"

#define TAG "fb-text"

#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE

/* Fonts */

#define FONTS_AMOUNT 20

const char* fontNames[] = {
	"freesans6", "freesans9", "freesansmono9", "freesansbold9", "freesansbold12",
	"org18",
	"fairlight8", "fairlight12",
	"dejavusans20",
	"permanentmarker22", "permanentmarker36",
	"roboto_black22", "roboto_blackitalic24", "roboto_regular12", "roboto_regular18", "roboto_regular22",
	"pixelade9", "pixelade13",
	"weather42",
	"7x5"
};
const GFXfont* fontPointers[] = {
	&freesans6pt7b, &freesans9pt7b, &freesansmono9pt7b, &freesansbold9pt7b, &freesansbold12pt7b,
	&org_018pt7b,
	&fairlight8pt7b, &fairlight12pt7b,
	&dejavusans20pt7b,
	&permanentmarker22pt7b, &permanentmarker36pt7b,
	&robotoblack22pt7b, &robotoblackitalic24pt7b, &roboto12pt7b, &roboto18pt7b, &roboto22pt7b,
	&pixelade9pt7b, &pixelade13pt7b,
	&weather42pt8b,
	&ipane7x5
};

/* Private functions */
void _print_char(Frame* frame, unsigned char c, int16_t x0, int16_t y0, uint8_t xScale, uint8_t yScale, uint32_t color, const GFXfont *font)
{
	if ((c < font->first) || (c > font->last)) {
		ESP_LOGE(TAG, "print_char called with unprintable character");
		return;
	}

	c -= (uint8_t) font->first;
	const GFXglyph *glyph   = font->glyph + c;
	const uint8_t  *bitmap  = font->bitmap;

	uint16_t bitmapOffset = glyph->bitmapOffset;
	uint8_t  width        = glyph->width;
	uint8_t  height       = glyph->height;
	int8_t   xOffset      = glyph->xOffset;
	int8_t   yOffset      = glyph->yOffset;

	uint8_t  bit = 0, bits = 0;
	for (uint8_t y = 0; y < height; y++) {
		for (uint8_t x = 0; x < width; x++) {
			if(!(bit++ & 7)) bits = bitmap[bitmapOffset++];
			if(bits & 0x80) {
				if (xScale == 1 && yScale == 1) {
					driver_framebuffer_setPixel(frame, x0+xOffset+x-1, y0+yOffset+y-1, color);
				} else {
					driver_framebuffer_rect(frame, x0+(xOffset+x)*xScale-1, y0+(yOffset+y)*yScale-1, xScale, yScale, true, color);
				}
			}
			bits <<= 1;
		}
	}
}

void _write(Frame* frame, uint8_t c, int16_t x0, int16_t *x, int16_t *y, uint8_t xScale, uint8_t yScale, uint32_t color, const GFXfont *font)
{
	if (font == NULL) { ESP_LOGE(TAG, "write called without font"); return; }
	const GFXglyph *glyph = font->glyph + c - (uint8_t) font->first;
	if (c == '\n') {
		*x = x0;
		*y += font->yAdvance * yScale;
	} else if (c != '\r') {
		_print_char(frame, c, *x, *y+(font->yAdvance*yScale), xScale, yScale, color, font);
		*x += glyph->xAdvance * xScale;
	}
}

uint16_t _char_width(uint8_t c, const GFXfont *font)
{
	if (font == NULL) return 0;
	if ((c < font->first) || (c > font->last)) return 0;
	const GFXglyph *glyph = font->glyph + c - (uint8_t) font->first;
	if ((c == '\r') || (c == '\n')) return 0;
	return glyph->xAdvance;
}

/* Public functions */

const GFXfont* driver_framebuffer_findFontByName(const char* fontName)
{
	char buffer[32];
	if (strlen(fontName) > 31) return false;
	strcpy(buffer, fontName);
	strlwr(buffer);
	for (uint16_t i = 0; i < FONTS_AMOUNT; i++) {
		if (strcmp(fontNames[i],buffer)==0) return fontPointers[i];
	}
	return NULL;
}

uint16_t driver_framebuffer_print(Frame* frame, const char* str, int16_t x0, int16_t y0, uint8_t xScale, uint8_t yScale, uint32_t color, const GFXfont *font)
{
	//printf("\nPrint text %s with color %u\n", str, color);
	int16_t x = x0, y = y0;
	for (uint16_t i = 0; i < strlen(str); i++) {
		_write(frame, str[i], x0, &x, &y, xScale, yScale, color, font);
	}
	return y;
}

uint16_t driver_framebuffer_print_len(Frame* frame, const char* str, int16_t len, int16_t x0, int16_t y0, uint8_t xScale, uint8_t yScale, uint32_t color, const GFXfont *font)
{
	int16_t x = x0, y = y0;
	for (uint16_t i = 0; i < len; i++) {
		_write(frame, str[i], x0, &x, &y, xScale, yScale, color, font);
	}
	return y;
}

uint16_t driver_framebuffer_get_string_width(const char* str, const GFXfont *font)
{
	uint16_t width = 0;
	for (uint16_t i = 0; i < strlen(str); i++) width += _char_width(str[i], font);
	return width;
}

uint16_t driver_framebuffer_get_string_height(const char* str, const GFXfont *font)
{
	uint16_t height = font->yAdvance;
	if (strlen(str) < 1) return 0;
	for (uint16_t i = 0; i < strlen(str)-1; i++) {
		if (str[i]=='\n') height += font->yAdvance;
	}
	return height;
}

#endif
