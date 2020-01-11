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

#define TAG "fb-drawing"

#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE

#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }

void driver_framebuffer_line(Window* window, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color)
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
			driver_framebuffer_setPixel(window, y0, x0, color);
		} else {
			driver_framebuffer_setPixel(window, x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

void driver_framebuffer_rect(Window* window, int16_t x, int16_t y, uint16_t w, uint16_t h, bool fill, uint32_t color)
{
	if (fill) {
		for (int16_t i=x; i<x+w; i++) {
			driver_framebuffer_line(window, i, y, i, y+h-1, color);
		}
	} else {
		driver_framebuffer_line(window, x,    y,     x+w-1, y,     color);
		driver_framebuffer_line(window, x,    y+h-1, x+w-1, y+h-1, color);
		driver_framebuffer_line(window, x,    y,     x,     y+h-1, color);
		driver_framebuffer_line(window, x+w-1,y,     x+w-1, y+h-1, color);
	}
}

void driver_framebuffer_circle(Window* window, int16_t x0, int16_t y0, uint16_t r, uint16_t a0, uint16_t a1, bool fill, uint32_t color)
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

	while (x<y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f     += ddF_y;
		}
		x++;
		ddF_x += 2;
		f     += ddF_x;

		if (fill) {
			//Please fix this part of the code, it doesn't work well.
			if (parts & (1<<0))         driver_framebuffer_line(window, x0, y0, x0 + x, y0 - y, color);
			if (parts & (1<<1))         driver_framebuffer_line(window, x0, y0, x0 + y, y0 - x, color);
			if (parts & (1<<2))         driver_framebuffer_line(window, x0, y0, x0 + y, y0 + x, color);
			if (parts & (1<<3))         driver_framebuffer_line(window, x0, y0, x0 + x, y0 + y, color);
			if (parts & (1<<4))         driver_framebuffer_line(window, x0, y0, x0 - x, y0 + y, color);
			if (parts & (1<<5))         driver_framebuffer_line(window, x0, y0, x0 - y, y0 + x, color);
			if (parts & (1<<6))         driver_framebuffer_line(window, x0, y0, x0 - y, y0 - x, color);
			if (parts & (1<<7))         driver_framebuffer_line(window, x0, y0, x0 - x, y0 - y, color);
			if (a0 == 0   || a1 == 360) driver_framebuffer_line(window, x0, y0, x0,     y0 - r, color);
			if (a0 <= 90  && a1 >=  90) driver_framebuffer_line(window, x0, y0, x0 + r, y0,     color);
			if (a0 <= 180 && a1 >= 180) driver_framebuffer_line(window, x0, y0, x0,     y0 + r, color);
			if (a0 <= 270 && a1 >= 270) driver_framebuffer_line(window, x0, y0, x0 - r, y0,     color);
		} else {
			//This only works up until 45 degree parts, for more control please rewrite this.
			if (parts & (1<<0))         driver_framebuffer_setPixel(window, x0 + x, y0 - y, color);
			if (parts & (1<<1))         driver_framebuffer_setPixel(window, x0 + y, y0 - x, color);
			if (parts & (1<<2))         driver_framebuffer_setPixel(window, x0 + y, y0 + x, color);
			if (parts & (1<<3))         driver_framebuffer_setPixel(window, x0 + x, y0 + y, color);
			if (parts & (1<<4))         driver_framebuffer_setPixel(window, x0 - x, y0 + y, color);
			if (parts & (1<<5))         driver_framebuffer_setPixel(window, x0 - y, y0 + x, color);
			if (parts & (1<<6))         driver_framebuffer_setPixel(window, x0 - y, y0 - x, color);
			if (parts & (1<<7))         driver_framebuffer_setPixel(window, x0 - x, y0 - y, color);
			if (a0 == 0   || a1 == 360) driver_framebuffer_setPixel(window, x0,     y0 - r, color);
			if (a0 <= 90  && a1 >=  90) driver_framebuffer_setPixel(window, x0 + r, y0,     color);
			if (a0 <= 180 && a1 >= 180) driver_framebuffer_setPixel(window, x0,     y0 + r, color);
			if (a0 <= 270 && a1 >= 270) driver_framebuffer_setPixel(window, x0 - r, y0,     color);
		}
	}
}

#endif
