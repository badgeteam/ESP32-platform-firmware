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

const GFXfont *gfxFont;
uint8_t textScaleX = 1;
uint8_t textScaleY = 1;
int16_t cursor_x = 0, cursor_x0 = 0, cursor_y = 0;
bool textWrap = true;
uint32_t textColor = COLOR_TEXT_DEFAULT;

/* Fonts */

#define FONTS_AMOUNT 19

const char* fontNames[] = {
	//NEW:
	"freesans6",
	"freesans9",
	"freesansmono9",
	"freesansbold9",
	"freesansbold12",
	"org18",
	"fairlight8",
	"fairlight12",
	"pixelade9",
	//SHA2017:
	"dejavusans20",
	"permanentmarker22",
	"permanentmarker36",
	"roboto_black22",
	"roboto_blackitalic24",
	"roboto_regular12",
	"roboto_regular18",
	"roboto_regular22",
	"pixelade13",
	"weather42"
};
const GFXfont* fontPointers[] = {
	//NEW:
	&freesans6pt7b,
	&freesans9pt7b,
	&freesansmono9pt7b,
	&freesansbold9pt7b,
	&freesansbold12pt7b,
	&org_018pt7b,
	&fairlight8pt7b,
	&fairlight12pt7b,
	&pixelade9pt7b,
	//SHA2017:
	&dejavusans20pt7b,
	&permanentmarker22pt7b,
	&permanentmarker36pt7b,
	&robotoblack22pt7b,
	&robotoblackitalic24pt7b,
	&roboto12pt7b,
	&roboto18pt7b,
	&roboto22pt7b,
	&pixelade13pt7b,
	&weather42pt8b
};

void driver_framebuffer_setFont(const GFXfont *font)
{
	gfxFont = font;
}

bool driver_framebuffer_selectFont(const char* fontName)
{
	char buffer[32];
	if (strlen(fontName) > 31) return false;
	strcpy(buffer, fontName);
	strlwr(buffer);
	for (uint16_t i = 0; i < FONTS_AMOUNT; i++) {
		if (strcmp(fontNames[i],buffer)==0) {
			driver_framebuffer_setFont(fontPointers[i]);
			return true;
		}
	}
	return false;
}

void driver_framebuffer_setCursor(int16_t x, int16_t y)
{
	cursor_x = x;
	cursor_x0 = x;
	cursor_y = y;
}

void driver_framebuffer_getCursor(int16_t* x, int16_t* y)
{
	*x = cursor_x;
	*y = cursor_y;
}

void driver_framebuffer_setTextScale(uint8_t w, uint8_t h)
{
	textScaleX = w;
	textScaleY = h;
}

void driver_framebuffer_getTextScale(uint8_t* w, uint8_t* h)
{
	*w = textScaleX;
	*h = textScaleY;
}

void driver_framebuffer_setTextColor(uint32_t value)
{
	textColor = value;
}

uint32_t driver_framebuffer_getTextColor()
{
	return textColor;
}

void print_char(int16_t x0, int16_t y0, unsigned char c, uint8_t xScale, uint8_t yScale, uint32_t color)
{
	if (gfxFont == NULL) return;
	if ((c < gfxFont->first) || (c > gfxFont->last)) return;

	c -= (uint8_t) gfxFont->first;
	const GFXglyph *glyph   = gfxFont->glyph + c;
	const uint8_t  *bitmap  = gfxFont->bitmap;

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
					driver_framebuffer_setPixel(NULL, x0+xOffset+x, y0+yOffset+y, color);
				} else {
					driver_framebuffer_rect(NULL, x0+(xOffset+x)*xScale, y0+(yOffset+y)*yScale, xScale, yScale, true, color);
				}
			}
			bits <<= 1;
		}
	}
}

void driver_framebuffer_write(uint8_t c)
{
	if (gfxFont == NULL) return;
	const GFXglyph *glyph = gfxFont->glyph + c - (uint8_t) gfxFont->first;
	if (c == '\n') {
		cursor_x = cursor_x0;
		cursor_y += gfxFont->yAdvance * textScaleY;
	} else if (c != '\r') {
		print_char(cursor_x, cursor_y+((gfxFont->yAdvance/2)*textScaleY), c, textScaleX, textScaleY, textColor);
		cursor_x += glyph->xAdvance * textScaleX;
	}
}

uint8_t driver_framebuffer_get_font_height()
{
	if (gfxFont == NULL) return 0;
	return gfxFont->yAdvance;
}

uint16_t driver_framebuffer_get_char_width(uint8_t c)
{
	if (gfxFont == NULL) return 0;
	if ((c < gfxFont->first) || (c > gfxFont->last)) return 0;
	const GFXglyph *glyph = gfxFont->glyph + c - (uint8_t) gfxFont->first;
	if ((c != '\r') && (c != '\n')) {
		return glyph->xAdvance * textScaleX;
	}
	return 0;
}

void driver_framebuffer_print(const char* str)
{
	for (uint16_t i = 0; i < strlen(str); i++) driver_framebuffer_write(str[i]);
}

uint16_t driver_framebuffer_get_string_width(const char* str)
{
	uint16_t width = 0;
	for (uint16_t i = 0; i < strlen(str); i++) width += driver_framebuffer_get_char_width(str[i]);
	return width;
}

uint16_t driver_framebuffer_get_string_height(const char* str)
{
	uint8_t lineHeight = driver_framebuffer_get_font_height();
	uint16_t height = lineHeight;
	if (strlen(str) < 1) return 0;
	for (uint16_t i = 0; i < strlen(str)-1; i++) {
		if (str[i]=='\n') height += lineHeight;
	}
	return height;
}

void driver_framebuffer_print_len(const char* str, int16_t len)
{
	for (uint16_t i = 0; i < len; i++) driver_framebuffer_write(str[i]);
}

#endif
