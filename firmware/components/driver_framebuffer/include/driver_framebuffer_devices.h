#ifndef _DRIVER_FRAMEBUFFER_DEVICES_H_
#define _DRIVER_FRAMEBUFFER_DEVICES_H_

#include "driver_ssd1306.h"
#include "driver_erc12864.h"
#include "driver_eink.h"
#include "driver_ili9341.h"

#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE

/* This file specifies the framebuffer configuration for the displays that are supported. */
/* The order in this file determines priority if multiple drivers are enabled */

/* E-INK display as used on the SHA2017 and HackerHotel 2019 badges */
#if defined(CONFIG_DRIVER_EINK_ENABLE)
	#define FB_SIZE EINK_BUFFER_SIZE
	#define FB_WIDTH DRIVER_EINK_WIDTH
	#define FB_HEIGHT DRIVER_EINK_HEIGHT
	#define FB_TYPE_8BPP
	#define FB_FLUSH(buffer,flags,x0,y0,x1,y1) driver_eink_display(buffer,flags);
	//#define FB_FLUSH(buffer,flags,x0,y0,x1,y1) driver_eink_display_part(buffer,flags,y0,y1);
	#define FB_FLUSH_GS(buffer,flags) driver_eink_display_greyscale(buffer,flags,16);
	#define COLOR_BLACK 0
	#define COLOR_WHITE 255

/* OLED display as used on the Disobey 2020 badge */
#elif defined(CONFIG_DRIVER_SSD1306_ENABLE)
	#define FB_SIZE SSD1306_BUFFER_SIZE
	#define FB_WIDTH SSD1306_WIDTH
	#define FB_HEIGHT SSD1306_HEIGHT
	#define FB_TYPE_1BPP
	#define FB_1BPP_VERT
	#define FB_FLUSH(buffer,flags,x0,y0,x1,y1) driver_ssd1306_write(buffer);
	#define COLOR_BLACK false
	#define COLOR_WHITE true

/* LCD display as used on the Disobey 2019 badge */
#elif defined(CONFIG_DRIVER_ERC12864_ENABLE)
	#define FB_SIZE ERC12864_BUFFER_SIZE
	#define FB_WIDTH ERC12864_WIDTH
	#define FB_HEIGHT ERC12864_HEIGHT
	#define FB_TYPE_1BPP
	#define FB_1BPP_VERT
	#define FB_FLUSH(buffer,flags,x0,y0,x1,y1) driver_erc12864_write(buffer);
	#define COLOR_BLACK false
	#define COLOR_WHITE true

/* LCD display as used on the Espressif Wrover kit */
#elif defined(CONFIG_DRIVER_ILI9341_ENABLE)
	#define FB_SIZE ILI9341_BUFFER_SIZE
	#define FB_WIDTH ILI9341_WIDTH
	#define FB_HEIGHT ILI9341_HEIGHT
	#define FB_TYPE_16BPP
	#define FB_FLUSH(buffer,flags,x0,y0,x1,y1) driver_ili9341_write_partial(buffer, x0, y0, x1, y1)
	#define COLOR_BLACK 0
	#define COLOR_WHITE 0xFFFF
	#define COLOR_FILL_DEFAULT COLOR_BLACK
	#define COLOR_TEXT_DEFAULT COLOR_WHITE

#else
#error "Framebuffer driver enabled without a target display available!"
#endif

#endif

#endif //_DRIVER_FRAMEBUFFER_DEVICES_H_
