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

#include "include/driver_framebuffer.h"
#include <stdio.h>
#include <string.h>

#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE

const GFXfont *gfxFont;
uint8_t fontHeight;
uint16_t textScaleX = 1;
uint16_t textScaleY = 1;

int16_t cursor_x = 0;
int16_t cursor_y = 0;
bool textWrap = true;
#ifdef FB_TYPE_1BPP
	bool textColor = false;
#endif
#ifdef FB_TYPE_8BPP
	uint8_t textColor = 0;
#endif
#ifdef FB_TYPE_24BPP
	uint8_t textColorR = 0, textColorG = 0, textColorB = 0;
#endif
	
uint8_t flags = 0;

void update_font_height()
{
	fontHeight = 0;
	for (uint8_t i = 0; i < gfxFont->last-gfxFont->first; i++) {
		GFXglyph *glyph = gfxFont->glyph + i;
		if (glyph->height > fontHeight) fontHeight = glyph->height;
	}
}

void driver_framebuffer_setFont(const GFXfont *font)
{
	gfxFont = font;
	update_font_height();
}

esp_err_t driver_framebuffer_init()
{
	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
	framebuffer1 = malloc(FB_SIZE);
	if (!framebuffer1) return ESP_FAIL;
	framebuffer2 = malloc(FB_SIZE);
	if (!framebuffer2) return ESP_FAIL;
	currentFb = false;
	#else
	framebuffer = malloc(FB_SIZE);
	if (!framebuffer) return ESP_FAIL;
	#endif
	driver_framebuffer_setFont(&freesans9pt7b);
	#if defined(FB_TYPE8BPP) && defined(DISPLAY_FLAG_8BITPIXEL)
		flags = DISPLAY_FLAG_8BITPIXEL;	
	#endif
	return ESP_OK;
}

#ifdef FB_TYPE_1BPP
void driver_framebuffer_fill(bool value)
{
	memset(framebuffer, value ? 0xFF : 0x00, FB_SIZE);
}

void driver_framebuffer_pixel(uint16_t x, uint16_t y, bool value)
{
	if (x >= FB_WIDTH) return;
	if (y >= FB_HEIGHT) return;
	#ifndef FB_1BPP_VERT
	uint32_t position = (y * FB_WIDTH) + (x / 8);
	uint8_t  bit      = x % 8;
	#else
	uint32_t position = ( (y / 8) * FB_WIDTH) + x;
	uint8_t  bit      = y % 8;
	#endif
	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
	uint8_t* framebuffer = currentFb ? framebuffer2 : framebuffer1;
	#endif
	framebuffer[position] ^= (-value ^ framebuffer[position]) & (1UL << bit);
}
#endif
#ifdef FB_TYPE_8BPP
void driver_framebuffer_fill(uint8_t value)
{
	memset(framebuffer, value, FB_SIZE);
}

void driver_framebuffer_pixel(uint16_t x, uint16_t y, uint8_t value)
{
	if (x >= FB_WIDTH) return;
	if (y >= FB_HEIGHT) return;
	uint32_t position = (y * FB_WIDTH) + x;
	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
	uint8_t* framebuffer = currentFb ? framebuffer2 : framebuffer1;
	#endif
	framebuffer[position] = value;
}
#endif
#ifdef FB_TYPE_24BPP
void driver_framebuffer_fill(uint8_t r, uint8_t g, uint8_t b)
{
	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
	uint8_t* framebuffer = currentFb ? framebuffer2 : framebuffer1;
	#endif
	for (uint32_t i = 0; i < FB_SIZE; i+=3) {
		framebuffer[i + 0] = r;
		framebuffer[i + 1] = g;
		framebuffer[i + 2] = b;
	}
}

void driver_framebuffer_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b)
{
	if (x >= FB_WIDTH) return;
	if (y >= FB_HEIGHT) return;
	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
	uint8_t* framebuffer = currentFb ? framebuffer2 : framebuffer1;
	#endif
	uint32_t position = (y * FB_WIDTH * 3) + (x * 3);
	framebuffer[position + 0] = r;
	framebuffer[position + 1] = g;
	framebuffer[position + 2] = b;
}
#endif

#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }

#ifdef FB_TYPE_1BPP
void driver_framebuffer_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, bool value)
#endif
#ifdef FB_TYPE_8BPP
void driver_framebuffer_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t value)
#endif
#ifdef FB_TYPE_24BPP
void driver_framebuffer_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t r, uint8_t g, uint8_t b)
#endif
{
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		_swap_int16_t(x0, y0);
		_swap_int16_t(x1, y1);
	}

	if (x0 > x1) {
		_swap_int16_t(x0, x1);
		_swap_int16_t(y0, y1);
	}
	
	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	for (/*empty*/; x0<=x1; x0++) {
		#if defined(FB_TYPE_1BPP) || defined(FB_TYPE_8BPP)
		if (steep) { driver_framebuffer_pixel(y0, x0, value); } else { driver_framebuffer_pixel(x0, y0, value); }
		#endif
		#ifdef FB_TYPE_24BPP
		if (steep) { driver_framebuffer_pixel(y0, x0, r, g, b); } else { driver_framebuffer_pixel(x0, y0, r, g, b); }
		#endif
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

#ifdef FB_TYPE_1BPP
void driver_framebuffer_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool fill, bool value)
#endif
#ifdef FB_TYPE_8BPP
void driver_framebuffer_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool fill, uint8_t value)
#endif
#ifdef FB_TYPE_24BPP
void driver_framebuffer_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool fill, uint8_t r, uint8_t g, uint8_t b)
#endif
{
	if (x > FB_WIDTH) return;
	if (y > FB_HEIGHT) return;
	if (x+w > FB_WIDTH) w = FB_WIDTH - x;
	if (y+h > FB_WIDTH) h = FB_HEIGHT - y;
	
	if (fill) {
		for (int16_t i=x; i<x+w; i++) {
			#if defined(FB_TYPE_1BPP) || defined(FB_TYPE_8BPP)
			driver_framebuffer_line(i, y, i, y+h, value);
			#endif
			#ifdef FB_TYPE_24BPP
			driver_framebuffer_line(i, y, i, y+h, r, g, b);
			#endif
		}
	} else {
		#if defined(FB_TYPE_1BPP) || defined(FB_TYPE_8BPP)
		driver_framebuffer_line(x,    y,     x+w-1, y,     value);
		driver_framebuffer_line(x,    y+h-1, x+w-1, y+h-1, value);
		driver_framebuffer_line(x,    y,     x,     y+h-1, value);
		driver_framebuffer_line(x+w-1,y,     x+w-1, y+h-1, value);
		#endif
		#ifdef FB_TYPE_24BPP
		driver_framebuffer_line(x,    y,     x+w-1, y,     r, g, b);
		driver_framebuffer_line(x,    y+h-1, x+w-1, y+h-1, r, g, b);
		driver_framebuffer_line(x,    y,     x,     y+h-1, r, g, b);
		driver_framebuffer_line(x+w-1,y,     x+w-1, y+h-1, r, g, b);
		#endif
	}
}

#ifdef FB_TYPE_1BPP
void driver_framebuffer_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t a0, uint16_t a1, bool fill, bool value)
#endif
#ifdef FB_TYPE_8BPP
void driver_framebuffer_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t a0, uint16_t a1, bool fill, uint8_t value)
#endif
#ifdef FB_TYPE_24BPP
void driver_framebuffer_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t a0, uint16_t a1, bool fill, uint8_t r, uint8_t g, uint8_t b)
#endif
{
	int16_t f     = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x     = 0;
	int16_t y     = r;
	
	if (a0 >= a1) return;
	
	uint8_t parts = 0;
	for (uint16_t i = 0, bit = 0; i<360; i+=45, bit++) {
		if (i>=a0 && i < a1) parts += 1<<bit;
	}
	
	for (uint8_t i = 0; i<8; i++) {
		printf("%s", (parts&(1<<i)) ? "1":"0");
	}
	printf("\n");

	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f     += ddF_y;
		}
		x++;
		ddF_x += 2;
		f     += ddF_x;
		
		printf("Circle x=%d, y=%d, f=%d, ddF_x=%d, ddF_y=%d\n", x, y, f, ddF_x, ddF_y);
		
		#if defined(FB_TYPE_1BPP) || defined(FB_TYPE_8BPP)
		if (fill) {
			//Please fix this part of the code, it doesn't work well.
			if (parts & (1<<0))         driver_framebuffer_line(x0, y0, x0 + x, y0 - y, value);
			if (parts & (1<<1))         driver_framebuffer_line(x0, y0, x0 + y, y0 - x, value);
			if (parts & (1<<2))         driver_framebuffer_line(x0, y0, x0 + y, y0 + x, value);
			if (parts & (1<<3))         driver_framebuffer_line(x0, y0, x0 + x, y0 + y, value);
			if (parts & (1<<4))         driver_framebuffer_line(x0, y0, x0 - x, y0 + y, value);
			if (parts & (1<<5))         driver_framebuffer_line(x0, y0, x0 - y, y0 + x, value);
			if (parts & (1<<6))         driver_framebuffer_line(x0, y0, x0 - y, y0 - x, value);
			if (parts & (1<<7))         driver_framebuffer_line(x0, y0, x0 - x, y0 - y, value);
			if (a0 == 0   || a1 == 360) driver_framebuffer_line(x0, y0, x0,     y0 - r, value);
			if (a0 <= 90  && a1 >=  90) driver_framebuffer_line(x0, y0, x0 + r, y0,     value);
			if (a0 <= 180 && a1 >= 180) driver_framebuffer_line(x0, y0, x0,     y0 + r, value);
			if (a0 <= 270 && a1 >= 270) driver_framebuffer_line(x0, y0, x0 - r, y0,     value);
		} else {
			//This only works up until 45 degree parts, for more control please rewrite this.
			if (parts & (1<<0))         driver_framebuffer_pixel(x0 + x, y0 - y, value);
			if (parts & (1<<1))         driver_framebuffer_pixel(x0 + y, y0 - x, value);
			if (parts & (1<<2))         driver_framebuffer_pixel(x0 + y, y0 + x, value);
			if (parts & (1<<3))         driver_framebuffer_pixel(x0 + x, y0 + y, value);
			if (parts & (1<<4))         driver_framebuffer_pixel(x0 - x, y0 + y, value);
			if (parts & (1<<5))         driver_framebuffer_pixel(x0 - y, y0 + x, value);
			if (parts & (1<<6))         driver_framebuffer_pixel(x0 - y, y0 - x, value);
			if (parts & (1<<7))         driver_framebuffer_pixel(x0 - x, y0 - y, value);
			if (a0 == 0   || a1 == 360) driver_framebuffer_pixel(x0,     y0 - r, value);
			if (a0 <= 90  && a1 >=  90) driver_framebuffer_pixel(x0 + r, y0,     value);
			if (a0 <= 180 && a1 >= 180) driver_framebuffer_pixel(x0,     y0 + r, value);
			if (a0 <= 270 && a1 >= 270) driver_framebuffer_pixel(x0 - r, y0,     value);
		}
		#endif
		#if defined(FB_TYPE_24BPP)
		if (fill) {
			//Please fix this part of the code, it doesn't work well.
			if (parts & (1<<0))         driver_framebuffer_line(x0, y0, x0 + x, y0 - y, r, g, b);
			if (parts & (1<<1))         driver_framebuffer_line(x0, y0, x0 + y, y0 - x, r, g, b);
			if (parts & (1<<2))         driver_framebuffer_line(x0, y0, x0 + y, y0 + x, r, g, b);
			if (parts & (1<<3))         driver_framebuffer_line(x0, y0, x0 + x, y0 + y, r, g, b);
			if (parts & (1<<4))         driver_framebuffer_line(x0, y0, x0 - x, y0 + y, r, g, b);
			if (parts & (1<<5))         driver_framebuffer_line(x0, y0, x0 - y, y0 + x, r, g, b);
			if (parts & (1<<6))         driver_framebuffer_line(x0, y0, x0 - y, y0 - x, r, g, b);
			if (parts & (1<<7))         driver_framebuffer_line(x0, y0, x0 - x, y0 - y, r, g, b);
			if (a0 == 0   || a1 == 360) driver_framebuffer_line(x0, y0, x0,     y0 - r, r, g, b);
			if (a0 <= 90  && a1 >=  90) driver_framebuffer_line(x0, y0, x0 + r, y0,     r, g, b);
			if (a0 <= 180 && a1 >= 180) driver_framebuffer_line(x0, y0, x0,     y0 + r, r, g, b);
			if (a0 <= 270 && a1 >= 270) driver_framebuffer_line(x0, y0, x0 - r, y0,     r, g, b);
		} else {
			//This only works up until 45 degree parts, for more control please rewrite this.
			if (parts & (1<<0))         driver_framebuffer_pixel(x0 + x, y0 - y, r, g, b);
			if (parts & (1<<1))         driver_framebuffer_pixel(x0 + y, y0 - x, r, g, b);
			if (parts & (1<<2))         driver_framebuffer_pixel(x0 + y, y0 + x, r, g, b);
			if (parts & (1<<3))         driver_framebuffer_pixel(x0 + x, y0 + y, r, g, b);
			if (parts & (1<<4))         driver_framebuffer_pixel(x0 - x, y0 + y, r, g, b);
			if (parts & (1<<5))         driver_framebuffer_pixel(x0 - y, y0 + x, r, g, b);
			if (parts & (1<<6))         driver_framebuffer_pixel(x0 - y, y0 - x, r, g, b);
			if (parts & (1<<7))         driver_framebuffer_pixel(x0 - x, y0 - y, r, g, b);
			if (a0 == 0   || a1 == 360) driver_framebuffer_pixel(x0,     y0 - r, r, g, b);
			if (a0 <= 90  && a1 >=  90) driver_framebuffer_pixel(x0 + r, y0,     r, g, b);
			if (a0 <= 180 && a1 >= 180) driver_framebuffer_pixel(x0,     y0 + r, r, g, b);
			if (a0 <= 270 && a1 >= 270) driver_framebuffer_pixel(x0 - r, y0,     r, g, b);
		}
		#endif
	}
}

#ifdef FB_TYPE_1BPP
void driver_framebuffer_char(uint16_t x0, uint16_t y0, unsigned char c, uint8_t xScale, uint8_t yScale, bool value)
#endif
#ifdef FB_TYPE_8BPP
void driver_framebuffer_char(uint16_t x0, uint16_t y0, unsigned char c, uint8_t xScale, uint8_t yScale, uint8_t value)
#endif
#ifdef FB_TYPE_24BPP
void driver_framebuffer_char(uint16_t x0, uint16_t y0, unsigned char c, uint8_t xScale, uint8_t yScale, uint8_t r, uint8_t g, uint8_t b)
#endif
{
	if (gfxFont == NULL) {
		printf("ATTEMPT TO CHAR WITH NULL FONT\n");
		return;
	}
	if ((c < gfxFont->first) || (c > gfxFont->last)) {
		printf("ATTEMPT TO CHAR WITH UNPRINTABLE CHARACTER\n");
		return;
	}
	
	c -= (uint8_t) gfxFont->first;
	const GFXglyph *glyph   = gfxFont->glyph + c;
	const uint8_t  *bitmap  = gfxFont->bitmap;
	
	uint16_t bitmapOffset = glyph->bitmapOffset;
	uint8_t  width        = glyph->width;
	uint8_t  height       = glyph->height;
	int8_t   xOffset      = glyph->xOffset;
	int8_t   yOffset      = glyph->yOffset;
	
	/*printf("bitmapOffset = %u\n", bitmapOffset);
	printf("width = %u\n", width);
	printf("height = %u\n", height);
	printf("xOffset = %d\n", xOffset);
	printf("yOffset = %d\n", yOffset);*/
	
	uint8_t  bit = 0, bits = 0;	
	for (uint8_t y = 0; y < height; y++) {
		for (uint8_t x = 0; x < width; x++) {
			if(!(bit++ & 7)) bits = bitmap[bitmapOffset++];
			//printf("char %u,%u = %02x\n", x, y, bits);
			if(bits & 0x80) {
				#if defined(FB_TYPE_1BPP) || defined(FB_TYPE_8BPP)
				if (xScale == 1 && yScale == 1) {
					driver_framebuffer_pixel(x0+xOffset+x, y0+yOffset+y+fontHeight, value);
				} else {
					driver_framebuffer_rect(x0+(xOffset+x)*xScale, y0+(yOffset+y+fontHeight)*yScale, xScale, yScale, true, value);
				}
				#endif
				#ifdef FB_TYPE_24BPP
				if (xScale == 1 && yScale == 1) {
					driver_framebuffer_pixel(x0+xOffset+x, y0+yOffset+y+fontHeight, r, g, b);
				} else {
					driver_framebuffer_rect(x0+(xOffset+x)*xScale, y0+(yOffset+y+fontHeight)*yScale, xScale, yScale, true, r, g, b);
				}
				#endif
			}
			bits <<= 1;
		}
	}
}

void driver_framebuffer_setCursor(int16_t x, int16_t y)
{
	cursor_x = x;
	cursor_y = y;
}

void driver_framebuffer_getCursor(int16_t* x, int16_t* y)
{
	*x = cursor_x;
	*y = cursor_y;
}

void driver_framebuffer_setScale(int16_t x, int16_t y)
{
	textScaleX = x;
	textScaleY = y;
}

#ifdef FB_TYPE_1BPP
void driver_framebuffer_setTextColor(bool value)
#endif
#ifdef FB_TYPE_8BPP
void driver_framebuffer_setTextColor(uint8_t value)
#endif
#ifdef FB_TYPE_24BPP
void driver_framebuffer_setTextColor(uint8_t r, uint8_t g, uint8_t b)
#endif
{
	#if defined(FB_TYPE_1BPP) || defined(FB_TYPE_8BPP)
	textColor = value;
	#endif
	#ifdef FB_TYPE_24BPP
	textColorR = r;
	textColorG = g;
	textColorB = b;
	#endif
}

void driver_framebuffer_write(uint8_t c)
{
	if (gfxFont == NULL) {
		printf("ATTEMPT TO WRITE WITH NULL FONT\n");
		return;
	}
	const GFXglyph *glyph = gfxFont->glyph + c - (uint8_t) gfxFont->first;
	/*uint8_t  width        = glyph->width;
	uint8_t  height       = glyph->height;
	int8_t   xOffset      = glyph->xOffset;
	int8_t   yOffset      = glyph->yOffset;*/
	uint8_t  xAdvance     = glyph->xAdvance;
	
	if (c == '\n') {
		cursor_x = 0;
		cursor_y += textScaleY * gfxFont->yAdvance;
	} else if (c != '\r') {
		#if defined(FB_TYPE_1BPP) || defined(FB_TYPE_8BPP)
		driver_framebuffer_char(cursor_x, cursor_y, c, textScaleX, textScaleY, textColor);
		#endif
		#ifdef FB_TYPE_24BPP
		driver_framebuffer_char(cursor_x, cursor_y, c, textScaleX, textScaleY, textColorR, textColorG, textColorB);
		#endif
		cursor_x += xAdvance * textScaleX;
	}
}

void driver_framebuffer_print(const char* str)
{
	for (uint16_t i = 0; i < strlen(str); i++) driver_framebuffer_write(str[i]);
}

void driver_framebuffer_print_len(const char* str, int16_t len)
{
	for (uint16_t i = 0; i < len; i++) driver_framebuffer_write(str[i]);
}

void driver_framebuffer_setFlags(uint8_t newFlags)
{
	flags = newFlags;
}

void driver_framebuffer_flush()
{
	uint8_t flags = 1; //Fox E-INK driver, needs to be replaced some day.
	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
	uint8_t* framebuffer = currentFb ? framebuffer2 : framebuffer1;
	uint8_t* nextFb = currentFb ? framebuffer1 : framebuffer2;
	currentFb = !currentFb; //Switch to the other framebuffer
	#endif
	FB_FLUSH(framebuffer,flags);
	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
	memcpy(nextFb, framebuffer, FB_SIZE); //Copy the framebuffer we just flushed into the working buffer
	#endif
}

#else
esp_err_t driver_framebuffer_init() { return ESP_OK; }
#endif
