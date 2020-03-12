/*
 * BADGE.TEAM framebuffer driver
 * Uses parts of the Adafruit GFX Arduino libray
 * Renze Nicolai 2019
 */

#include "sdkconfig.h"
#include "include/driver_framebuffer_internal.h"

#define TAG "fb"

#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE

#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
uint8_t* framebuffer1;
uint8_t* framebuffer2;
#endif

uint8_t* framebuffer;

/* Color space conversions */

inline uint16_t convert24to16(uint32_t in) //RGB24 to 565
{
#ifdef CONFIG_DRIVER_FRAMEBUFFER_SWAP_R_AND_B
	uint8_t b = (in>>16)&0xFF;
	uint8_t r = in&0xFF;
#else
	uint8_t r = (in>>16)&0xFF;
	uint8_t b = in&0xFF;    
#endif
	uint8_t g = (in>>8)&0xFF;
	return ((b & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (r >> 3);
}

inline uint8_t convert24to8C(uint32_t in) //RGB24 to 256-color
{
#ifdef CONFIG_DRIVER_FRAMEBUFFER_SWAP_R_AND_B
	uint8_t r = ((in>>16)&0xFF) >> 5;
	uint8_t b = ( in     &0xFF) >> 6;
#else
	uint8_t b = ((in>>16)&0xFF) >> 5;
	uint8_t r = ( in     &0xFF) >> 6;
#endif
	uint8_t g = ((in>> 8)&0xFF) >> 5;
	return r | (g<<3) | (b<<6);
}

inline uint32_t convert8Cto24(uint8_t in) //256-color to RGB24
{
#ifdef CONFIG_DRIVER_FRAMEBUFFER_SWAP_R_AND_B
	uint8_t b = in & 0x07;
	uint8_t r = in >> 6;
#else
	uint8_t r = in & 0x07;
	uint8_t b = in >> 6;
#endif
	uint8_t g = (in>>3) & 0x07;
	return b | (g << 8) | (r << 16);
}

inline uint8_t convert24to8(uint32_t in) //RGB24 to 8-bit greyscale
{
#ifdef CONFIG_DRIVER_FRAMEBUFFER_SWAP_R_AND_B
	uint8_t b = (in>>16)&0xFF;
	uint8_t r = in&0xFF;
#else
	uint8_t r = (in>>16)&0xFF;
	uint8_t b = in&0xFF;
#endif
	uint8_t g = (in>>8)&0xFF;
	return ( r + g + b + 1 ) / 3;
}

inline bool convert8to1(uint8_t in) //8-bit greyscale to black&white
{
	return in >= 128;
}

esp_err_t driver_framebuffer_init()
{
	static bool driver_framebuffer_init_done = false;
	if (driver_framebuffer_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
		ESP_LOGI(TAG, "Allocating %u bytes for framebuffer 1", FB_SIZE);
		#ifdef CONFIG_DRIVER_FRAMEBUFFER_SPIRAM
			framebuffer1 = (uint8_t*) heap_caps_malloc(FB_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
		#else
			framebuffer1 = (uint8_t*) heap_caps_malloc(FB_SIZE, MALLOC_CAP_8BIT);
		#endif
		if (!framebuffer1) return ESP_FAIL;
		ESP_LOGI(TAG, "Allocating %u bytes for framebuffer 2", FB_SIZE);
		#ifdef CONFIG_DRIVER_FRAMEBUFFER_SPIRAM
			framebuffer2 = (uint8_t*) heap_caps_malloc(FB_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
		#else
			framebuffer2 = (uint8_t*) heap_caps_malloc(FB_SIZE, MALLOC_CAP_8BIT);
		#endif
		if (!framebuffer2) return ESP_FAIL;
		framebuffer = framebuffer1;
	#else
		ESP_LOGI(TAG, "Allocating %u bytes for the framebuffer", FB_SIZE);
		#ifdef CONFIG_DRIVER_FRAMEBUFFER_SPIRAM
		framebuffer = (uint8_t*) heap_caps_malloc(FB_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
		#else
		framebuffer = (uint8_t*) heap_caps_malloc(FB_SIZE, MALLOC_CAP_8BIT);
		#endif
		if (!framebuffer) {
			ESP_LOGE(TAG, "Unable to allocate memory for the framebuffer.");
			return ESP_FAIL;
		}
	#endif
		
	driver_framebuffer_fill(NULL, COLOR_FILL_DEFAULT); //1st framebuffer
	
	#ifdef CONFIG_DRIVER_HUB75_ENABLE
		driver_hub75_switch_buffer(framebuffer); //Needed to make the legacy compositor work.
	#endif
	
	//driver_framebuffer_flush(FB_FLAG_FORCE | FB_FLAG_FULL);
	//driver_framebuffer_fill(NULL, COLOR_FILL_DEFAULT); //2nd framebuffer
	driver_framebuffer_set_orientation_angle(NULL, 0); //Apply global orientation (needed for flip)
	driver_framebuffer_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

bool _getFrameContext(Window* window, uint8_t** buffer, int16_t* width, int16_t* height)
{
	if (window == NULL) {
		//No window provided, use global context
		*width = FB_WIDTH;
		*height = FB_HEIGHT;
		*buffer = framebuffer;
		if (!framebuffer) {
			ESP_LOGE(TAG, "Framebuffer not allocated!");
			return false;
		}
	} else {
		*width  = window->width;
		*height = window->height;
		*buffer = window->buffer;
	}
	return true;
}

void driver_framebuffer_fill(Window* window, uint32_t value)
{
	uint8_t* buffer;
	int16_t width, height;
	if (!_getFrameContext(window, &buffer, &width, &height)) return;
	if (!window) driver_framebuffer_set_dirty_area(0,0,width-1,height-1, true);
	
	#if   defined(FB_TYPE_1BPP)
		memset(buffer, convert8to1(convert24to8(value)) ? 0xFF : 0x00, (width*height)/8);
	#elif defined(FB_TYPE_8BPP)
		memset(buffer, convert24to8(value), width*height);
	#elif defined(FB_TYPE_12BPP)
		#ifdef CONFIG_DRIVER_FRAMEBUFFER_SWAP_R_AND_B
			uint8_t r = (value >> 20) &0x0F;
			uint8_t b = (value >> 04) &0x0F;
		#else
			uint8_t b = (value >> 20) &0x0F;
			uint8_t r = (value >> 04) &0x0F;
		#endif
		uint8_t g = (value >> 12) &0x0F;
		for (uint32_t position = 0; position < width*height; position++) {
			#elif defined(FB_TYPE_12BPP)
			uint32_t positionBits = (x+(y*width))*12;
			uint32_t positionByte = positionBits/8;
			uint8_t positionBit = positionBits % 8;
			switch(positionBit) {
				case 0:
					buffer[positionByte+0] = (r<<4) | g;
					buffer[positionByte+1] = (b<<4) | (buffer[positionByte+1]&0x0F);
					break;
				case 4:
					buffer[positionByte+0] = (buffer[positionByte+0]&0xF0) | r;
					buffer[positionByte+1] = (g<<4) | b;
					break;
				default:
					printf("??? %u, %u: %u = %u(%u)\n", x,y,positionBits, positionByte, positionBit);
			}
		}
	#elif defined(FB_TYPE_16BPP)
		value = convert24to16(value);
		uint8_t c0 = (value>>8)&0xFF;
		uint8_t c1 = value&0xFF;
		for (uint32_t i = 0; i < width*height*2; i+=2) {
			buffer[i + 0] = c0;
			buffer[i + 1] = c1;
		}
	#elif defined(FB_TYPE_8CBPP)
		value = convert24to8C(value);
		for (uint32_t i = 0; i < width*height; i++) {
			buffer[i] = value;
		}
	#elif defined(FB_TYPE_24BPP)
		#ifdef CONFIG_DRIVER_FRAMEBUFFER_SWAP_R_AND_B
			uint8_t b = (value>>16)&0xFF;
			uint8_t r = value&0xFF;
		#else
			uint8_t r = (value>>16)&0xFF;
			uint8_t b = value&0xFF;
		#endif
		uint8_t g = (value>>8)&0xFF;
		for (uint32_t i = 0; i < width*height*3; i+=3) {
			buffer[i + 0] = r;
			buffer[i + 1] = g;
			buffer[i + 2] = b;
		}
	#elif defined(FB_TYPE_32BPP)
		#ifdef CONFIG_DRIVER_FRAMEBUFFER_SWAP_R_AND_B
			uint8_t b = (value>>16)&0xFF;
			uint8_t r = value&0xFF;
		#else
			uint8_t r = (value>>16)&0xFF;
			uint8_t b = value&0xFF;
		#endif
		uint8_t g = (value>>8)&0xFF;
		uint8_t a = (value>>24)&0xFF;
		for (uint32_t i = 0; i < width*height*4; i+=4) {
			buffer[i + 0] = a;
			buffer[i + 1] = b;
			buffer[i + 2] = g;
			buffer[i + 3] = r;
		}
	#else
		#error "No framebuffer type configured."
	#endif
}

void driver_framebuffer_setPixel(Window* window, int16_t x, int16_t y, uint32_t value)
{
	uint8_t* buffer; int16_t width, height;
	if (!_getFrameContext(window, &buffer, &width, &height)) return;
	if (!driver_framebuffer_orientation_apply(window, &x, &y)) return;
	bool changed = false;
	#if defined(FB_TYPE_1BPP)
		value = convert8to1(convert24to8(value));
		#if defined(FB_1BPP_VERT)
			// A byte consists of 8 vertical pixels,
			// each byte is placed next to each other horizontally
			uint32_t position = ( (y / 8) * width) + x;
			uint8_t  bit      = y % 8;
		#elif defined(FB_1BPP_VERT2)
			// TODO: description
			uint32_t position = (y/8) + (x*height/8);
			uint8_t  bit      = y % 8;
		#elif defined(FB_1BPP_OHS)
			// TODO: description
			uint32_t position = ((width-x-1) + y * width) / 8;
			uint8_t  bit      = x % 8;
		#else
			// TODO: description
			uint32_t position = (y * (width/8)) + (x / 8);
			uint8_t  bit      = x % 8;
		#endif

		uint8_t oldVal = buffer[position];
		if (value) {
			buffer[position] |= 1UL << bit;
		} else {
			buffer[position] &= ~(1UL << bit);
		}
		if (oldVal != buffer[position]) changed = true;
	#elif defined(FB_TYPE_8BPP)
		value = convert24to8(value);
		uint32_t position = (y * width) + x;
		if (buffer[position] != value) changed = true;
		buffer[position] = value;
	#elif defined(FB_TYPE_12BPP)
		//12-bit (RRRRGGGGBBBB)
		uint32_t positionBits = (x+(y*width))*12;
		uint32_t positionByte = positionBits/8;
		uint8_t positionBit = positionBits % 8;
		
		uint8_t r = (value >> 20) &0x0F;
		uint8_t g = (value >> 12) &0x0F;
		uint8_t b = (value >> 04) &0x0F;
		
		switch(positionBit) {
			case 0:
				buffer[positionByte+0] = (r<<4) | g;
				buffer[positionByte+1] = (b<<4) | (buffer[positionByte+1]&0x0F);
				break;
			case 4:
				buffer[positionByte+0] = (buffer[positionByte+0]&0xF0) | r;
				buffer[positionByte+1] = (g<<4) | b;
				break;
			default:
				printf("??? %u, %u: %u = %u(%u)\n", x,y,positionBits, positionByte, positionBit);
		}
	#elif defined(FB_TYPE_16BPP)
		value = convert24to16(value);
		uint8_t c0 = (value>>8)&0xFF;
		uint8_t c1 = value&0xFF;
		uint32_t position = (y * width * 2) + (x * 2);
		if (buffer[position + 0] != c0 || buffer[position + 1] != c1) changed = true;
		buffer[position + 0] = c0;
		buffer[position + 1] = c1;
	#elif defined(FB_TYPE_8CBPP)
		uint32_t position = (y * width) + x;
		value = convert24to8C(value);
		if (value != buffer[position]) changed = true;
		buffer[position] = value;
	#elif defined(FB_TYPE_24BPP)
		uint8_t r = (value>>16)&0xFF;
		uint8_t g = (value>>8)&0xFF;
		uint8_t b = value&0xFF;
		uint32_t position = (y * width * 3) + (x * 3);
		if (buffer[position + 0] != r || buffer[position + 1] != g || buffer[position + 2] != b) changed = true;
		buffer[position + 0] = r;
		buffer[position + 1] = g;
		buffer[position + 2] = b;
	#elif defined(FB_TYPE_32BPP)
		uint8_t a = (value>>24)&0xFF;
		uint8_t r = (value>>16)&0xFF;
		uint8_t g = (value>>8)&0xFF;
		uint8_t b = value&0xFF;
		uint32_t position = (y * width * 4) + (x * 4);
		if (buffer[position + 0] != a || buffer[position + 1] != b || buffer[position + 2] != g || buffer[position + 3] != r) changed = true;
		buffer[position + 0] = a;
		buffer[position + 1] = b;
		buffer[position + 2] = g;
		buffer[position + 3] = r;
	#else
		#error "No framebuffer type configured."
	#endif
	
	if ((!window) && changed) driver_framebuffer_set_dirty_area(x,y,x,y,false);
}

uint32_t driver_framebuffer_getPixel(Window* window, int16_t x, int16_t y)
{
	uint8_t* buffer; int16_t width, height;
	if (!_getFrameContext(window, &buffer, &width, &height)) return 0;
	if (!driver_framebuffer_orientation_apply(window, &x, &y)) return 0;

	#if defined(FB_TYPE_1BPP)
		#ifndef FB_1BPP_VERT
			#if defined(FB_1BPP_OHS)
			#warning "USING OHS!!"
				uint32_t position = ((width-x-1) + y * width) / 8;
				uint8_t  bit      = x % 8;
			#elif defined(FB_1BPP_VERT2)
				#warning "USING VERT2!"
				uint32_t position = (y/8) + (x*height/8);
				uint8_t  bit      = y % 8;
			#else
			#warning "USING HORI!"
				uint32_t position = (y * (width/8)) + (x / 8);
				uint8_t  bit      = x % 8;
			#endif
		#else
			#warning "USING VERT!"
			uint32_t position = ( (y / 8) * width) + x;
			uint8_t  bit      = y % 8;
		#endif
		if ((buffer[position] >> bit) & 0x01) {
			return 0xFFFFFF;
		} else {
			return 0x000000;
		}
	#elif defined(FB_TYPE_8BPP)
		uint32_t position = (y * width) + x;
		return (buffer[position] << 16) + (buffer[position]<<8) + buffer[position];
	#elif defined(FB_TYPE_12BPP)
		//12-bit (RRRRGGGGBBBB)
		uint32_t positionBits = (x+(y*width))*12;
		uint32_t positionByte = positionBits/8;
		uint8_t positionBit = positionBits % 8;
		
		uint8_t r = 0;
		uint8_t g = 0;
		uint8_t b = 0;
		
		switch(positionBit) {
			case 0:
				r = (buffer[positionByte+0] & 0xF0);
				g = (buffer[positionByte+0] << 4);
				b = (buffer[positionByte+1] & 0xF0);
				break;
			case 4:
				r = (buffer[positionByte+0] << 4);
				g = (buffer[positionByte+1] & 0xF0);
				b = (buffer[positionByte+1] << 4);
				break;
			default:
				printf("??? %u, %u: %u = %u(%u)\n", x,y,positionBits, positionByte, positionBit);
		}
	#elif defined(FB_TYPE_16BPP)
		uint32_t position = (y * width * 2) + (x * 2);
		uint32_t color = (buffer[position] << 8) + (buffer[position + 1]);
		uint8_t r = ((((color >> 11) & 0x1F) * 527) + 23) >> 6;
		uint8_t g = ((((color >> 5 ) & 0x3F) * 259) + 33) >> 6;
		uint8_t b = ((((color      ) & 0x1F) * 527) + 23) >> 6;
		return r << 16 | g << 8 | b;
	#elif defined(FB_TYPE_8CBPP)
		return convert8Cto24(buffer[(y * width) + x]);
	#elif defined(FB_TYPE_24BPP)
		uint32_t position = (y * width * 3) + (x * 3);
		return (buffer[position+2] << 16) + (buffer[position+1] << 8) + (buffer[position + 0]);
	#elif defined(FB_TYPE_32BPP)
		uint32_t position = (y * width * 4) + (x * 4);
		return (buffer[position] << 24) + (buffer[position+3] << 16) + (buffer[position+2] << 8) + (buffer[position+1]);
	#else
		#error "No framebuffer type configured."
	#endif
}

void driver_framebuffer_blit(Window* source, Window* target)
{
	if (source->vOffset >= source->height) return; //The vertical offset is larger than the height of the window
	if (source->hOffset >= source->width)  return; //The horizontal offset is larger than the width of the window
	for (uint16_t wy = source->vOffset; wy < source->drawHeight; wy++) {
		for (uint16_t wx = source->hOffset; wx < source->drawWidth; wx++) {
			if (wy >= source->height) continue; //Out-of-bounds
			if (wx >= source->width) continue;  //Out-of-bounds
			uint32_t color = driver_framebuffer_getPixel(source, wx, wy); //Read the pixel from the window framebuffer
			if (source->enableTransparentColor && source->transparentColor == color) continue; //Transparent
			driver_framebuffer_setPixel(target, source->x + wx, source->y + wy, color); //Write the pixel to the global framebuffer
		}
	}
}

void _render_windows()
{
	//Step through the linked list of windows and blit each of the visible windows to the main framebuffer
	Window* currentWindow = driver_framebuffer_window_first();
	while (currentWindow != NULL) {
		if (currentWindow->visible) {
			driver_framebuffer_blit(currentWindow, NULL);
		}
		currentWindow = currentWindow->_nextWindow;
	}
}

bool driver_framebuffer_flush(uint32_t flags)
{
	if (!framebuffer) {
		ESP_LOGE(TAG, "flush without alloc!");
		return false;
	}
	
	_render_windows();

	uint32_t eink_flags = 0;
	
	if ((flags & FB_FLAG_FULL) || (flags & FB_FLAG_FORCE)) {
		driver_framebuffer_set_dirty_area(0, 0, FB_WIDTH-1, FB_HEIGHT-1, true);
		#ifdef DISPLAY_FLAG_LUT_BIT
			eink_flags |= DRIVER_EINK_LUT_FULL << DISPLAY_FLAG_LUT_BIT;
		#endif
	} else if (!driver_framebuffer_is_dirty()) {
		return false; //No need to update, stop.
	}
	
	#ifdef CONFIG_DRIVER_HUB75_ENABLE
	compositor_disable();
	#endif
	
	#if defined(FB_TYPE_8BPP) && defined(DISPLAY_FLAG_8BITPIXEL)
		eink_flags |= DISPLAY_FLAG_8BITPIXEL;
	#endif

	#ifdef DISPLAY_FLAG_LUT_BIT
		if (flags & FB_FLAG_LUT_NORMAL) {
			eink_flags |= DRIVER_EINK_LUT_NORMAL << DISPLAY_FLAG_LUT_BIT;
		}
		if (flags & FB_FLAG_LUT_FAST) {
			eink_flags |= DRIVER_EINK_LUT_FASTER << DISPLAY_FLAG_LUT_BIT;
		}
		if (flags & FB_FLAG_LUT_FASTEST) {
			eink_flags |= DRIVER_EINK_LUT_FASTEST << DISPLAY_FLAG_LUT_BIT;
		}
#else
#error "NO LUT BIT"
	#endif
		
	#ifdef FB_FLUSH_GS
	if (flags & FB_FLAG_LUT_GREYSCALE) {
		FB_FLUSH_GS(framebuffer, eink_flags);
	} else {
	#endif
		int16_t dirty_x0, dirty_y0, dirty_x1, dirty_y1;
		driver_framebuffer_get_dirty_area(&dirty_x0, &dirty_y0, &dirty_x1, &dirty_y1);
		FB_FLUSH(framebuffer,eink_flags,dirty_x0,dirty_y0,dirty_x1,dirty_y1);
	#ifdef FB_FLUSH_GS
	}
	#endif
	
	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
	if (framebuffer==framebuffer1) {
		framebuffer = framebuffer2;
		memcpy(framebuffer, framebuffer1, FB_SIZE);
	} else {
		framebuffer = framebuffer1;
		memcpy(framebuffer, framebuffer2, FB_SIZE);
	}
	#endif

	driver_framebuffer_set_dirty_area(FB_WIDTH-1, FB_HEIGHT-1, 0, 0, true); //Not dirty.
	return true;
}

esp_err_t driver_framebuffer_png(Window* window, int16_t x, int16_t y, lib_reader_read_t reader, void* reader_p)
{
	if (!framebuffer) {
		ESP_LOGE(TAG, "png without alloc!");
		return ESP_FAIL;
	}
	struct lib_png_reader *pr = lib_png_new(reader, reader_p);
	if (pr == NULL) {
		printf("Out of memory.\n");
		return ESP_FAIL;
	}
	
	int res = lib_png_read_header(pr);
	if (res < 0) {
		lib_png_destroy(pr);
		printf("Can not read header.\n");
		return ESP_FAIL;
	}
	
	int width = pr->ihdr.width;
	int height = pr->ihdr.height;	
	
	uint32_t dst_min_x = x < 0 ? -x : 0;
	uint32_t dst_min_y = y < 0 ? -y : 0;
	
	int16_t screenWidth;
	int16_t screenHeight;
		
	driver_framebuffer_get_orientation_size(window, &screenWidth, &screenHeight);
	
	res = lib_png_load_image(window, pr, x, y, dst_min_x, dst_min_y, screenWidth - x, screenHeight - y, screenWidth);

	lib_png_destroy(pr);

	if (res < 0) {
		printf("Failed to load image.\n");
		return ESP_FAIL;
	}
	
	driver_framebuffer_set_dirty_area(x, y, x + width - 1, y + height - 1, false);
	return ESP_OK;
}

uint16_t driver_framebuffer_getWidth(Window* window)
{
	int16_t width, height;
	driver_framebuffer_get_orientation_size(window, &width, &height);
	return width;
}

uint16_t driver_framebuffer_getHeight(Window* window)
{
	int16_t width, height;
	driver_framebuffer_get_orientation_size(window, &width, &height);
	return height;
}


uint8_t currentBrightness = 0;
esp_err_t driver_framebuffer_setBacklight(uint8_t brightness)
{
	#if defined(FB_SET_BACKLIGHT)
		currentBrightness = brightness;
		return FB_SET_BACKLIGHT(brightness);
	#else
		return ESP_FAIL;
	#endif
}

uint8_t driver_framebuffer_getBacklight()
{
	return currentBrightness;
}

#else
esp_err_t driver_framebuffer_init() { return ESP_OK; }
#endif
