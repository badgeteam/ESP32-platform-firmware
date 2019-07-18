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
#include "include/driver_framebuffer_devices.h"

//PNG library
#include "mem_reader.h"
#include "file_reader.h"
#include "png_reader.h"

//Displays
#include "driver_ssd1306.h"
#include "driver_erc12864.h"
#include "driver_eink.h"
#include "driver_ili9341.h"

#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE

const GFXfont *gfxFont;
uint8_t fontHeight;
uint16_t textScaleX = 1;
uint16_t textScaleY = 1;
int16_t cursor_x = 0;
int16_t cursor_y = 0;
bool textWrap = true;
uint32_t textColor = COLOR_BLACK;

bool isDirty = true;
int16_t dirty_x0 = 0;
int16_t dirty_y0 = 0;
int16_t dirty_x1 = 0;
int16_t dirty_y1 = 0;

uint8_t flags = 0; //Used to pass extra information to the display driver (if supported)

#define ORIENTATION_LANDSCAPE 0
#define ORIENTATION_PORTRAIT  1

bool orientation = ORIENTATION_LANDSCAPE;
bool flip180     = false;
bool useGreyscale = false;

/* Color space conversions */

inline uint16_t rgbTo565(uint32_t in)
{
	uint8_t r = (in>>16)&0xFF;
	uint8_t g = (in>>8)&0xFF;
	uint8_t b = in&0xFF;
	uint16_t out = ((b & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (r >> 3);
	return out;
}

inline uint8_t rgbToGrey(uint32_t in)
{
	uint8_t r = (in>>16)&0xFF;
	uint8_t g = (in>>8)&0xFF;
	uint8_t b = in&0xFF;
	return ( r + g + b + 1 ) / 3;
}

inline bool greyToBw(uint8_t in)
{
	return in >= 128;
}

bool driver_framebuffer_is_dirty()
{
	return isDirty;
}

void driver_framebuffer_set_greyscale(bool use)
{
	useGreyscale = use;
	isDirty = true;
	dirty_x0 = 0;
	dirty_y0 = 0;
	dirty_x1 = FB_WIDTH;
	dirty_y1 = FB_HEIGHT;
}


uint16_t driver_framebuffer_get_orientation()
{
	return orientation*90 + flip180*180;
}

void driver_framebuffer_set_orientation(uint16_t angle)
{
	switch(angle) {
		case 90:
			orientation = ORIENTATION_PORTRAIT;
			flip180     = false;
			break;
		case 180:
			orientation = ORIENTATION_LANDSCAPE;
			flip180     = true;
			break;
		case 270:
			orientation = ORIENTATION_PORTRAIT;
			flip180     = true;
			break;
		default:
			orientation = ORIENTATION_LANDSCAPE;
			flip180     = false;
			break;
	}
}

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
	#if defined(FB_TYPE8_BPP) && defined(DISPLAY_FLAG_8BITPIXEL)
		flags = DISPLAY_FLAG_8BITPIXEL;	
	#endif
	
	driver_framebuffer_fill(COLOR_WHITE);
	
	return ESP_OK;
}

#if defined(FB_TYPE_1BPP)
void driver_framebuffer_fill(uint32_t value)
{
	value = greyToBw(rgbToGrey(value));
	isDirty = true;
	dirty_x0 = 0;
	dirty_y0 = 0;
	dirty_x1 = FB_WIDTH;
	dirty_y1 = FB_HEIGHT;
	memset(framebuffer, value ? 0xFF : 0x00, FB_SIZE);
}

void driver_framebuffer_pixel(int16_t x, int16_t y, uint32_t value)
{
	value = greyToBw(rgbToGrey(value));
	if (orientation) { int16_t t = y; y = x; x = FB_WIDTH-t-1; }
	if (flip180)     { y = FB_HEIGHT-y-1;    x = FB_WIDTH-x-1; }
	if (x >= FB_WIDTH) return;
	if (x < 0) return;
	if (y >= FB_HEIGHT) return;
	if (y < 0) return;
	
	isDirty = true;
	if (x < dirty_x0) dirty_x0 = x;
	if (y < dirty_y0) dirty_y0 = y;
	if (x > dirty_x1) dirty_x1 = x;
	if (y > dirty_y1) dirty_y1 = y;

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

uint32_t driver_framebuffer_getPixel(int16_t x, int16_t y)
{
	if (orientation) { int16_t t = y; y = x; x = FB_WIDTH-t-1; }
	if (flip180)     { y = FB_HEIGHT-y-1;    x = FB_WIDTH-x-1; }
	if (x >= FB_WIDTH) return 0;
	if (x < 0) return 0;
	if (y >= FB_HEIGHT) return 0;
	if (y < 0) return 0;

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
	if ((framebuffer[position] >> bit) && 0x01) {
		return 0xFFFFFF;
	} else {
		return 0x000000;
	}
}
#elif defined(FB_TYPE_8BPP)
void driver_framebuffer_fill(uint32_t value)
{
	value = rgbToGrey(value);
	isDirty = true;
	dirty_x0 = 0;
	dirty_y0 = 0;
	dirty_x1 = FB_WIDTH;
	dirty_y1 = FB_HEIGHT;
	memset(framebuffer, value, FB_SIZE);
}

void driver_framebuffer_pixel(int16_t x, int16_t y, uint32_t value)
{
	value = rgbToGrey(value);
	if (orientation) { int16_t t = y; y = x; x = FB_WIDTH-t-1; }
	if (flip180)     { y = FB_HEIGHT-y-1;    x = FB_WIDTH-x-1; }
	if (x >= FB_WIDTH) return;
	if (x < 0) return;
	if (y >= FB_HEIGHT) return;
	if (y < 0) return;

	isDirty = true;
	if (x < dirty_x0) dirty_x0 = x;
	if (y < dirty_y0) dirty_y0 = y;
	if (x > dirty_x1) dirty_x1 = x;
	if (y > dirty_y1) dirty_y1 = y;

	uint32_t position = (y * FB_WIDTH) + x;
	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
		uint8_t* framebuffer = currentFb ? framebuffer2 : framebuffer1;
	#endif
	framebuffer[position] = value;
}

uint32_t driver_framebuffer_getPixel(int16_t x, int16_t y)
{
	if (orientation) { int16_t t = y; y = x; x = FB_WIDTH-t-1; }
	if (flip180)     { y = FB_HEIGHT-y-1;    x = FB_WIDTH-x-1; }
	if (x >= FB_WIDTH) return 0;
	if (x < 0) return 0;
	if (y >= FB_HEIGHT) return 0;
	if (y < 0) return 0;

	uint32_t position = (y * FB_WIDTH) + x;
	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
		uint8_t* framebuffer = currentFb ? framebuffer2 : framebuffer1;
	#endif
	return (framebuffer[position] << 16) + (framebuffer[position]<<8) + framebuffer[position];
}
#elif defined(FB_TYPE_16BPP)
void driver_framebuffer_fill(uint32_t color)
{
	color = rgbTo565(color);
	isDirty = true;
	dirty_x0 = 0;
	dirty_y0 = 0;
	dirty_x1 = FB_WIDTH;
	dirty_y1 = FB_HEIGHT;
	uint8_t c0 = (color>>8)&0xFF;
	uint8_t c1 = color&0xFF;
	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
		uint8_t* framebuffer = currentFb ? framebuffer2 : framebuffer1;
	#endif
	for (uint32_t i = 0; i < FB_SIZE; i+=2) {
		framebuffer[i + 0] = c0;
		framebuffer[i + 1] = c1;
	}
}

void driver_framebuffer_pixel(int16_t x, int16_t y, uint32_t color)
{
	color = rgbTo565(color);
	uint8_t c0 = (color>>8)&0xFF;
	uint8_t c1 = color&0xFF;

	if (orientation) { int16_t t = y; y = x; x = FB_WIDTH-t-1; }
	if (flip180)     { y = FB_HEIGHT-y-1;    x = FB_WIDTH-x-1; }
	if (x >= FB_WIDTH) return;
	if (x < 0) return;
	if (y >= FB_HEIGHT) return;
	if (y < 0) return;

	isDirty = true;
	if (x < dirty_x0) dirty_x0 = x;
	if (y < dirty_y0) dirty_y0 = y;
	if (x > dirty_x1) dirty_x1 = x;
	if (y > dirty_y1) dirty_y1 = y;

	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
		uint8_t* framebuffer = currentFb ? framebuffer2 : framebuffer1;
	#endif
	uint32_t position = (y * FB_WIDTH * 2) + (x * 2);
	framebuffer[position + 0] = c0;
	framebuffer[position + 1] = c1;
}
uint32_t driver_framebuffer_getPixel(int16_t x, int16_t y)
{
	if (orientation) { int16_t t = y; y = x; x = FB_WIDTH-t-1; }
	if (flip180)     { y = FB_HEIGHT-y-1;    x = FB_WIDTH-x-1; }
	if (x >= FB_WIDTH) return 0;
	if (x < 0) return 0;
	if (y >= FB_HEIGHT) return 0;
	if (y < 0) return 0;

	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
		uint8_t* framebuffer = currentFb ? framebuffer2 : framebuffer1;
	#endif
	uint32_t position = (y * FB_WIDTH * 2) + (x * 2);
	uint32_t color = (framebuffer[position] << 8) + (framebuffer[position + 1]);
	uint8_t r = ((((color >> 11) & 0x1F) * 527) + 23) >> 6;

	uint8_t g = ((((color >> 5) & 0x3F) * 259) + 33) >> 6;

	uint8_t b = (((color & 0x1F) * 527) + 23) >> 6;
	return r << 16 | g << 8 | b;;
}
#elif defined(FB_TYPE_24BPP)
void driver_framebuffer_fill(uint32_t color)
{
	isDirty = true;
	dirty_x0 = 0;
	dirty_y0 = 0;
	dirty_x1 = FB_WIDTH;
	dirty_y1 = FB_HEIGHT;
	uint8_t r = (color>>16)&0xFF;
	uint8_t g = (color>>8)&0xFF;
	uint8_t b = color&0xFF;
	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
		uint8_t* framebuffer = currentFb ? framebuffer2 : framebuffer1;
	#endif
	for (uint32_t i = 0; i < FB_SIZE; i+=3) {
		framebuffer[i + 0] = r;
		framebuffer[i + 1] = g;
		framebuffer[i + 2] = b;
	}
}

void driver_framebuffer_pixel(int16_t x, int16_t y, uint32_t color)
{
        uint8_t r = (color>>16)&0xFF;
        uint8_t g = (color>>8)&0xFF;
        uint8_t b = color&0xFF;

	if (orientation) { int16_t t = y; y = x; x = FB_WIDTH-t-1; }
	if (flip180)     { y = FB_HEIGHT-y-1;    x = FB_WIDTH-x-1; }
	if (x >= FB_WIDTH) return;
	if (x < 0) return;
	if (y >= FB_HEIGHT) return;
	if (y < 0) return;

	isDirty = true;
	if (x < dirty_x0) dirty_x0 = x;
	if (y < dirty_y0) dirty_y0 = y;
	if (x > dirty_x1) dirty_x1 = x;
	if (y > dirty_y1) dirty_y1 = y;

	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
		uint8_t* framebuffer = currentFb ? framebuffer2 : framebuffer1;
	#endif
	uint32_t position = (y * FB_WIDTH * 3) + (x * 3);
	framebuffer[position + 0] = r;
	framebuffer[position + 1] = g;
	framebuffer[position + 2] = b;
}
uint32_t driver_framebuffer_getPixel(int16_t x, int16_t y)
{
	if (orientation) { int16_t t = y; y = x; x = FB_WIDTH-t-1; }
	if (flip180)     { y = FB_HEIGHT-y-1;    x = FB_WIDTH-x-1; }
	if (x >= FB_WIDTH) return 0;
	if (x < 0) return 0;
	if (y >= FB_HEIGHT) return 0;
	if (y < 0) return 0;

	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
		uint8_t* framebuffer = currentFb ? framebuffer2 : framebuffer1;
	#endif
	uint32_t position = (y * FB_WIDTH * 3) + (x * 3);
	return (framebuffer[position] << 16) + (framebuffer[position+1] << 8) + (framebuffer[position + 2]);
}
#else
#error "Framebuffer can not be enabled without valid output configuration."
#endif

#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }

void driver_framebuffer_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color)
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
		if (steep) {
			driver_framebuffer_pixel(y0, x0, color);
		} else {
			driver_framebuffer_pixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

void driver_framebuffer_rect(int16_t x, int16_t y, uint16_t w, uint16_t h, bool fill, uint32_t color)
{
	if (x > FB_WIDTH) return;
	if (y > FB_HEIGHT) return;
	if (x+w > FB_WIDTH) w = FB_WIDTH - x;
	if (y+h > FB_WIDTH) h = FB_HEIGHT - y;
	
	if (fill) {
		for (int16_t i=x; i<x+w; i++) {
			driver_framebuffer_line(i, y, i, y+h, color);
		}
	} else {
		driver_framebuffer_line(x,    y,     x+w-1, y,     color);
		driver_framebuffer_line(x,    y+h-1, x+w-1, y+h-1, color);
		driver_framebuffer_line(x,    y,     x,     y+h-1, color);
		driver_framebuffer_line(x+w-1,y,     x+w-1, y+h-1, color);
	}
}

void driver_framebuffer_circle(int16_t x0, int16_t y0, uint16_t r, uint16_t a0, uint16_t a1, bool fill, uint32_t color)
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
		
		if (fill) {
			//Please fix this part of the code, it doesn't work well.
			if (parts & (1<<0))         driver_framebuffer_line(x0, y0, x0 + x, y0 - y, color);
			if (parts & (1<<1))         driver_framebuffer_line(x0, y0, x0 + y, y0 - x, color);
			if (parts & (1<<2))         driver_framebuffer_line(x0, y0, x0 + y, y0 + x, color);
			if (parts & (1<<3))         driver_framebuffer_line(x0, y0, x0 + x, y0 + y, color);
			if (parts & (1<<4))         driver_framebuffer_line(x0, y0, x0 - x, y0 + y, color);
			if (parts & (1<<5))         driver_framebuffer_line(x0, y0, x0 - y, y0 + x, color);
			if (parts & (1<<6))         driver_framebuffer_line(x0, y0, x0 - y, y0 - x, color);
			if (parts & (1<<7))         driver_framebuffer_line(x0, y0, x0 - x, y0 - y, color);
			if (a0 == 0   || a1 == 360) driver_framebuffer_line(x0, y0, x0,     y0 - r, color);
			if (a0 <= 90  && a1 >=  90) driver_framebuffer_line(x0, y0, x0 + r, y0,     color);
			if (a0 <= 180 && a1 >= 180) driver_framebuffer_line(x0, y0, x0,     y0 + r, color);
			if (a0 <= 270 && a1 >= 270) driver_framebuffer_line(x0, y0, x0 - r, y0,     color);
		} else {
			//This only works up until 45 degree parts, for more control please rewrite this.
			if (parts & (1<<0))         driver_framebuffer_pixel(x0 + x, y0 - y, color);
			if (parts & (1<<1))         driver_framebuffer_pixel(x0 + y, y0 - x, color);
			if (parts & (1<<2))         driver_framebuffer_pixel(x0 + y, y0 + x, color);
			if (parts & (1<<3))         driver_framebuffer_pixel(x0 + x, y0 + y, color);
			if (parts & (1<<4))         driver_framebuffer_pixel(x0 - x, y0 + y, color);
			if (parts & (1<<5))         driver_framebuffer_pixel(x0 - y, y0 + x, color);
			if (parts & (1<<6))         driver_framebuffer_pixel(x0 - y, y0 - x, color);
			if (parts & (1<<7))         driver_framebuffer_pixel(x0 - x, y0 - y, color);
			if (a0 == 0   || a1 == 360) driver_framebuffer_pixel(x0,     y0 - r, color);
			if (a0 <= 90  && a1 >=  90) driver_framebuffer_pixel(x0 + r, y0,     color);
			if (a0 <= 180 && a1 >= 180) driver_framebuffer_pixel(x0,     y0 + r, color);
			if (a0 <= 270 && a1 >= 270) driver_framebuffer_pixel(x0 - r, y0,     color);
		}
	}
}

void driver_framebuffer_char(int16_t x0, int16_t y0, unsigned char c, uint8_t xScale, uint8_t yScale, uint32_t color)
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
				if (xScale == 1 && yScale == 1) {
					driver_framebuffer_pixel(x0+xOffset+x, y0+yOffset+y+fontHeight, color);
				} else {
					driver_framebuffer_rect(x0+(xOffset+x)*xScale, y0+(yOffset+y+fontHeight)*yScale, xScale, yScale, true, color);
				}
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

void driver_framebuffer_setTextColor(uint32_t value)
{
	textColor = value;
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
		driver_framebuffer_char(cursor_x, cursor_y, c, textScaleX, textScaleY, textColor);
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

void driver_framebuffer_get_dirty(int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1)
{
	*x0 = dirty_x0;
	*y0 = dirty_y0;
	*x1 = dirty_x1;
	*y1 = dirty_y1;

	if (flip180) {
		int16_t tx0 = *x0;
		int16_t ty0 = *y0;
		*x0 = FB_WIDTH-*x1-1;
		*y0 = FB_HEIGHT-*y1-1;
		*x1 = FB_WIDTH-tx0-1;
		*y1 = FB_HEIGHT-ty0-1;
	}

	if (orientation) {
		int16_t tx0 = *x0;
		int16_t tx1 = *x1;
		int16_t ty1 = *y1;
		*x0 = *y0;
		*y0 = FB_WIDTH-tx1-1;
		*x1 = ty1;
		*y1 = FB_WIDTH-tx0-1;
	}
}

void driver_framebuffer_flush()
{
	if (!isDirty) return; //No need to update

	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
	uint8_t* framebuffer = currentFb ? framebuffer2 : framebuffer1;
	uint8_t* nextFb = currentFb ? framebuffer1 : framebuffer2;
	currentFb = !currentFb; //Switch to the other framebuffer
	#endif

	#ifdef DISPLAY_FLAG_8BITPIXEL
		flags |= DISPLAY_FLAG_8BITPIXEL;
	#endif

	if (dirty_x0 < 0) dirty_x0 = 0;
	if (dirty_x0 > FB_WIDTH) dirty_x0 = FB_WIDTH;
	if (dirty_x1 < 0) dirty_x1 = 0;
	if (dirty_x1 > FB_WIDTH) dirty_x1 = FB_WIDTH;
	if (dirty_y0 < 0) dirty_y0 = 0;
	if (dirty_y0 > FB_WIDTH) dirty_y0 = FB_HEIGHT;
	if (dirty_y1 < 0) dirty_y1 = 0;
	if (dirty_y1 > FB_WIDTH) dirty_y1 = FB_HEIGHT;
	
	dirty_y1 +=1; //Temp. fix

	#ifdef FB_FLUSH_GS
		if (useGreyscale) {
			FB_FLUSH_GS(framebuffer, flags);
		} else {
			FB_FLUSH(framebuffer,flags,dirty_x0,dirty_y0,dirty_x1,dirty_y1);
		}
	#else
		FB_FLUSH(framebuffer,flags,dirty_x0,dirty_y0,dirty_x1,dirty_y1);
	#endif
	
	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
	memcpy(nextFb, framebuffer, FB_SIZE); //Copy the framebuffer we just flushed into the working buffer
	#endif

	dirty_x0 = FB_WIDTH;
	dirty_y0 = FB_HEIGHT;
	dirty_x1 = 0;
	dirty_y1 = 0;
	isDirty = false;
}


esp_err_t driver_framebuffer_png(int16_t x, int16_t y, const uint8_t* png_data, size_t len)
{
	if (x >= FB_WIDTH || y >= FB_HEIGHT)
 {
		printf("PNG too large!\n");
		return ESP_FAIL;
	}

	lib_reader_read_t reader;
	void * reader_p;

	struct lib_mem_reader *mr = lib_mem_new(png_data, len);
	if (mr == NULL) {
		printf("Out of memory 1\n");
		return ESP_FAIL;
	}
	
	reader = (lib_reader_read_t) &lib_mem_read;
	reader_p = mr;

	struct lib_png_reader *pr = lib_png_new(reader, reader_p);
	if (pr == NULL) {
		lib_mem_destroy(reader_p);
		printf("Out of memory 2\n");
		return ESP_FAIL;
	}
	
	int res = lib_png_read_header(pr);
	if (res < 0) {
		lib_png_destroy(pr);
		lib_mem_destroy(reader_p);
		printf("Can not read header.");
		return ESP_FAIL;
	}
	
	int width = pr->ihdr.width;
	int height = pr->ihdr.height;
	int bit_depth = pr->ihdr.bit_depth;
	int color_type = pr->ihdr.color_type;
	
	isDirty = true;
	if (x < dirty_x0) dirty_x0 = x;
	if (y < dirty_y0) dirty_y0 = y;
	if (x+width > dirty_x1) dirty_x1 = x+width;
	if (y+height > dirty_y1) dirty_y1 = y+height;
	
	uint32_t dst_min_x = x < 0 ? -x : 0;
	uint32_t dst_min_y = y < 0 ? -y : 0;
	
	uint8_t bitsPerPixel = 0;
	#if defined(FB_TYPE_24BPP)
	bitsPerPixel = 24;
	#elif defined(FB_TYPE_8BPP)
	bitsPerPixel = 8;
	#elif defined(FB_TYPE_1BPP)
	bitsPerPixel = 1;
	#endif
		
	res = lib_png_load_image(pr, x, y, dst_min_x, dst_min_y, FB_WIDTH - x, FB_HEIGHT - y, FB_WIDTH, 8);

	lib_png_destroy(pr);
	lib_mem_destroy(reader_p);

	if (res < 0) {
		printf("Failed to load image.\n");
		return ESP_FAIL;
	}
	
	return ESP_OK;
}

int16_t driver_framebuffer_getWidth(void)
{
	return orientation ? FB_HEIGHT : FB_WIDTH;
}
int16_t driver_framebuffer_getHeight(void)
{
	return orientation ? FB_WIDTH : FB_HEIGHT;
}

#else
esp_err_t driver_framebuffer_init() { return ESP_OK; }
#endif
