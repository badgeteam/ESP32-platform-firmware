#ifndef _DRIVER_FRAMEBUFFER_H_
#define _DRIVER_FRAMEBUFFER_H_
#include "driver_ssd1306.h"
#include "driver_erc12864.h"
#include "driver_eink.h"

#ifdef CONFIG_DRIVER_SSD1306_ENABLE
	#define FB_SIZE SSD1306_BUFFER_SIZE
	#define FB_WIDTH SSD1306_WIDTH
	#define FB_HEIGHT SSD1306_HEIGHT
	#define FB_TYPE_1BPP
	#define FB_1BPP_VERT
	#define FB_FLUSH(buffer,flags,x0,y0,x1,y1) driver_ssd1306_write(buffer);
	#define COLOR_BLACK 0
	#define COLOR_WHITE 1
#endif

#ifdef CONFIG_DRIVER_ERC12864_ENABLE
	#define FB_SIZE ERC12864_BUFFER_SIZE
	#define FB_WIDTH ERC12864_WIDTH
	#define FB_HEIGHT ERC12864_HEIGHT
	#define FB_TYPE_1BPP
	#define FB_1BPP_VERT
	#define FB_FLUSH(buffer,flags,x0,y0,x1,y1) driver_erc12864_write(buffer);
	#define COLOR_BLACK 0
	#define COLOR_WHITE 1
#endif

#ifdef CONFIG_DRIVER_EINK_ENABLE
	#define FB_SIZE EINK_BUFFER_SIZE
	#define FB_WIDTH DRIVER_EINK_WIDTH
	#define FB_HEIGHT DRIVER_EINK_HEIGHT
	#define FB_TYPE_8BPP
	//#define FB_FLUSH(buffer,flags) driver_eink_display(buffer,flags);
	#define FB_FLUSH(buffer,flags,x0,y0,x1,y1) driver_eink_display_part(buffer,flags,y0,y1);
	#define FB_FLUSH_GS(buffer,flags) driver_eink_display_greyscale(buffer,flags,16);
	#define COLOR_BLACK 0
	#define COLOR_WHITE 255
#endif

#include "gfxfont.h"

#ifndef PROGMEM //We don't use PROGMEM.
	#define PROGMEM
#endif

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

bool driver_framebuffer_is_dirty();
void driver_framebuffer_set_greyscale(bool use);

void driver_framebuffer_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t color);
void driver_framebuffer_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool fill, uint32_t color);
void driver_framebuffer_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t a0, uint16_t a1, bool fill, uint32_t color);
void driver_framebuffer_char(uint16_t x0, uint16_t y0, unsigned char c, uint8_t xScale, uint8_t yScale, uint32_t color);
void driver_framebuffer_setTextColor(uint32_t color);


#ifdef FB_TYPE_1BPP
void driver_framebuffer_fill(bool value);
void driver_framebuffer_pixel(uint16_t x, uint16_t y, bool value);
bool driver_framebuffer_getPixel(uint16_t x, uint16_t y);
#endif
#ifdef FB_TYPE_8BPP
void driver_framebuffer_fill(uint8_t value);
void driver_framebuffer_pixel(uint16_t x, uint16_t y, uint8_t value);
uint8_t driver_framebuffer_getPixel(uint16_t x, uint16_t y);
#endif
#ifdef FB_TYPE_24BPP
void driver_framebuffer_fill(uint8_t r, uint8_t g, uint8_t b);
void driver_framebuffer_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);
uint32_t driver_framebuffer_getPixel(uint16_t x, uint16_t y);
#endif

#endif //_DRIVER_FRAMEBUFFER_H_
