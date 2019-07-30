/* This file specifies the framebuffer configuration for the displays that are supported. */
/* The order in this file determines priority if multiple drivers are enabled */

#ifndef _DRIVER_FRAMEBUFFER_DEVICES_H_
#define _DRIVER_FRAMEBUFFER_DEVICES_H_

#include "driver_ssd1306.h"
#include "driver_erc12864.h"
#include "driver_eink.h"
#include "driver_ili9341.h"
#include "driver_hub75.h"

/* E-INK display as used on the SHA2017 and HackerHotel 2019 badges */
#if defined(CONFIG_DRIVER_EINK_ENABLE)
	#define FB_SIZE EINK_BUFFER_SIZE
	#define FB_WIDTH DRIVER_EINK_WIDTH
	#define FB_HEIGHT DRIVER_EINK_HEIGHT
	#define FB_TYPE_8BPP
	#define FB_ALPHA_ENABLED
	#define FB_FLUSH(buffer,eink_flags,x0,y0,x1,y1) driver_eink_display(buffer,eink_flags);
	//#define FB_FLUSH(buffer,eink_flags,x0,y0,x1,y1) driver_eink_display_part(buffer,eink_flags,y0,y1);
	#define FB_FLUSH_GS(buffer,eink_flags) driver_eink_display_greyscale(buffer,eink_flags,16);
	#define COLOR_FILL_DEFAULT 0xFFFFFF
	#define COLOR_TEXT_DEFAULT 0x000000
	
/* OLED display as used on the Disobey 2020 badge */
#elif defined(CONFIG_DRIVER_SSD1306_ENABLE)
	#define FB_SIZE SSD1306_BUFFER_SIZE
	#define FB_WIDTH SSD1306_WIDTH
	#define FB_HEIGHT SSD1306_HEIGHT
	#define FB_TYPE_1BPP
	#define FB_1BPP_VERT
	#define FB_FLUSH(buffer,eink_flags,x0,y0,x1,y1) driver_ssd1306_write(buffer);
	#define COLOR_FILL_DEFAULT 0x000000
	#define COLOR_TEXT_DEFAULT 0xFFFFFF

/* LCD display as used on the Disobey 2019 badge */
#elif defined(CONFIG_DRIVER_ERC12864_ENABLE)
	#define FB_SIZE ERC12864_BUFFER_SIZE
	#define FB_WIDTH ERC12864_WIDTH
	#define FB_HEIGHT ERC12864_HEIGHT
	#define FB_TYPE_1BPP
	#define FB_1BPP_VERT
	#define FB_FLUSH(buffer,eink_flags,x0,y0,x1,y1) driver_erc12864_write(buffer);
	#define COLOR_FILL_DEFAULT 0x000000
	#define COLOR_TEXT_DEFAULT 0xFFFFFF

/* LCD display as used on the Espressif Wrover kit */
#elif defined(CONFIG_DRIVER_ILI9341_ENABLE)
	#define FB_SIZE ILI9341_BUFFER_SIZE
	#define FB_WIDTH ILI9341_WIDTH
	#define FB_HEIGHT ILI9341_HEIGHT
	#define FB_TYPE_16BPP
	#define FB_ALPHA_ENABLED
	#define FB_FLUSH(buffer,eink_flags,x0,y0,x1,y1) driver_ili9341_write_partial(buffer, x0, y0, x1, y1)
	#define COLOR_FILL_DEFAULT 0x000000
	#define COLOR_TEXT_DEFAULT 0xFFFFFF

/* HUB75 led matrix */
#elif defined(CONFIG_DRIVER_HUB75_ENABLE)
	#define FB_SIZE HUB75_BUFFER_SIZE
	#define FB_WIDTH HUB75_WIDTH
	#define FB_HEIGHT HUB75_HEIGHT
	#define FB_TYPE_24BPP
	#define FB_ALPHA_ENABLED
	#define FB_FLUSH(buffer,eink_flags,x0,y0,x1,y1) driver_hub75_switch_buffer(buffer)
	#define COLOR_FILL_DEFAULT 0x000000
	#define COLOR_TEXT_DEFAULT 0xFFFFFF
#else
	#error "Framebuffer driver enabled without a target display available!"
#endif

#if defined(FB_TYPE_1BPP)
	#define PIXEL_SIZE 1
#elif defined(FB_TYPE_8BPP)
	#define PIXEL_SIZE 8
#elif defined(FB_TYPE_16BPP)
	#define PIXEL_SIZE 16
#elif defined(FB_TYPE_24BPP)
	#define PIXEL_SIZE 24
#endif

#endif //_DRIVER_FRAMEBUFFER_DEVICES_H_
