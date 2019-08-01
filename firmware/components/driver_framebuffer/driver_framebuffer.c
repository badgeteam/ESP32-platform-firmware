/*
 * BADGE.TEAM framebuffer driver
 * Uses parts of the Adafruit GFX Arduino libray
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

esp_err_t driver_framebuffer_init()
{
	static bool driver_framebuffer_init_done = false;
	if (driver_framebuffer_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	#ifdef CONFIG_DRIVER_FRAMEBUFFER_DOUBLE_BUFFERED
		ESP_LOGI(TAG, "Allocating %u bytes for framebuffer 1", FB_SIZE);
		#ifdef CONFIG_DRIVER_FRAMEBUFFER_SPIRAM
			framebuffer1 = heap_caps_malloc(FB_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
		#else
			framebuffer1 = heap_caps_malloc(FB_SIZE, MALLOC_CAP_8BIT);
		#endif
		if (!framebuffer1) return ESP_FAIL;
		ESP_LOGI(TAG, "Allocating %u bytes for framebuffer 2", FB_SIZE);
		#ifdef CONFIG_DRIVER_FRAMEBUFFER_SPIRAM
			framebuffer2 = heap_caps_malloc(FB_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
		#else
			framebuffer2 = heap_caps_malloc(FB_SIZE, MALLOC_CAP_8BIT);
		#endif
		if (!framebuffer2) return ESP_FAIL;
		framebuffer = framebuffer1;
	#else
		ESP_LOGI(TAG, "Allocating %u bytes for the framebuffer", FB_SIZE);
		#ifdef CONFIG_DRIVER_FRAMEBUFFER_SPIRAM
		framebuffer = heap_caps_malloc(FB_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
		#else
		framebuffer = heap_caps_malloc(FB_SIZE, MALLOC_CAP_8BIT);
		#endif
		
		if (!framebuffer) {
			ESP_LOGE(TAG, "Unable to allocate memory for the framebuffer.");
			return ESP_FAIL;
		}
	#endif
	
	driver_framebuffer_setFont(&freesans9pt7b);
		
	driver_framebuffer_fill(NULL, COLOR_FILL_DEFAULT); //1st framebuffer
	driver_framebuffer_setTextColor(COLOR_TEXT_DEFAULT);
	driver_framebuffer_flush(FB_FLAG_FORCE | FB_FLAG_FULL);
	driver_framebuffer_fill(NULL, COLOR_FILL_DEFAULT); //2nd framebuffer
	
	/* Window test */
	Window* testWindow = driver_framebuffer_create_window("testWindow", 50, 50);
	testWindow->visible = true;
	driver_framebuffer_fill(testWindow->frames, 0xFFFFFF);
	driver_framebuffer_rect(testWindow->frames, 0, 0, testWindow->width, testWindow->height, false, 0x000000);
	driver_framebuffer_line(testWindow->frames, testWindow->width-1, 0, 0, testWindow->height-1, 0x000000);
	
	testWindow = driver_framebuffer_create_window("testWindow #2", 50, 50);
	testWindow->x = 20;
	testWindow->y = 20;
	testWindow->visible = true;
	driver_framebuffer_fill(testWindow->frames, 0xFFFFFF);
	driver_framebuffer_rect(testWindow->frames, 0, 0, testWindow->width, testWindow->height, false, 0x000000);
	driver_framebuffer_line(testWindow->frames, 0, 0, testWindow->width-1, testWindow->height-1, 0x000000);
	
	//testWindow = driver_framebuffer_find_window("testWindow");
	//driver_framebuffer_focus_window(testWindow);
	/* ----------- */
	
	driver_framebuffer_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

bool _getFrameContext(Frame* frame, uint8_t** buffer, int16_t* width, int16_t* height)
{
	if (frame == NULL) {
		//No frame provided, use global context
		*width = FB_WIDTH;
		*height = FB_HEIGHT;
		*buffer = framebuffer;
		if (!framebuffer) {
			ESP_LOGE(TAG, "Framebuffer not allocated!");
			return false;
		}
	} else {
		if (frame->window == NULL) {
			ESP_LOGE(TAG, "Window is NULL!");
			return false;
		}
		*width  = frame->window->width;
		*height = frame->window->height;
		*buffer = frame->buffer;
	}
	return true;
}

void driver_framebuffer_fill(Frame* frame, uint32_t value)
{
	uint8_t* buffer;
	int16_t width, height;
	if (!_getFrameContext(frame, &buffer, &width, &height)) return;
	if (!frame) driver_framebuffer_set_dirty_area(0,0,width-1,height-1, true);
	
	#if   defined(FB_TYPE_1BPP)
		memset(buffer, greyToBw(rgbToGrey(value)) ? 0xFF : 0x00, (width*height)/8);
	#elif defined(FB_TYPE_8BPP)
		memset(buffer, rgbToGrey(value), width*height);
	#elif defined(FB_TYPE_16BPP)
		value = rgbTo565(value);
		uint8_t c0 = (value>>8)&0xFF;
		uint8_t c1 = value&0xFF;
		for (uint32_t i = 0; i < width*height*2; i+=2) {
			buffer[i + 0] = c0;
			buffer[i + 1] = c1;
		}
	#elif defined(FB_TYPE_24BPP)
		uint8_t r = (value>>16)&0xFF;
		uint8_t g = (value>>8)&0xFF;
		uint8_t b = value&0xFF;
		for (uint32_t i = 0; i < width*height*3; i+=3) {
			buffer[i + 0] = r;
			buffer[i + 1] = g;
			buffer[i + 2] = b;
		}
	#elif defined(FB_TYPE_32BPP)
		uint8_t a = (value>>24)&0xFF;
		uint8_t r = (value>>16)&0xFF;
		uint8_t g = (value>>8)&0xFF;
		uint8_t b = value&0xFF;
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

void driver_framebuffer_setPixel(Frame* frame, int16_t x, int16_t y, uint32_t value)
{
	uint8_t* buffer; int16_t width, height;
	if (!_getFrameContext(frame, &buffer, &width, &height)) return;
	if (!driver_framebuffer_orientation_apply(frame ? frame->window : NULL, &x, &y)) return;
	if (!frame) driver_framebuffer_set_dirty_area(x,y,x,y,false);
	
	#if defined(FB_TYPE_1BPP)
		value = greyToBw(rgbToGrey(value));
		#ifndef FB_1BPP_VERT
			uint32_t position = (y * width) + (x / 8);
			uint8_t  bit      = x % 8;
		#else
			uint32_t position = ( (y / 8) * width) + x;
			uint8_t  bit      = y % 8;
		#endif
		buffer[position] ^= (-value ^ buffer[position]) & (1UL << bit);
	#elif defined(FB_TYPE_8BPP)
		value = rgbToGrey(value);
		uint32_t position = (y * width) + x;
		buffer[position] = value;
	#elif defined(FB_TYPE_16BPP)
		value = rgbTo565(value);
		uint8_t c0 = (value>>8)&0xFF;
		uint8_t c1 = value&0xFF;
		uint32_t position = (y * width * 2) + (x * 2);
		buffer[position + 0] = c0;
		buffer[position + 1] = c1;
	#elif defined(FB_TYPE_24BPP)
		uint8_t r = (value>>16)&0xFF;
		uint8_t g = (value>>8)&0xFF;
		uint8_t b = value&0xFF;
		uint32_t position = (y * width * 3) + (x * 3);
		buffer[position + 0] = r;
		buffer[position + 1] = g;
		buffer[position + 2] = b;
	#elif defined(FB_TYPE_32BPP)
		uint8_t a = (value>>24)&0xFF;
		uint8_t r = (value>>16)&0xFF;
		uint8_t g = (value>>8)&0xFF;
		uint8_t b = value&0xFF;
		uint32_t position = (y * width * 4) + (x * 4);
		buffer[position + 0] = r;
		buffer[position + 1] = g;
		buffer[position + 2] = b;
		buffer[position + 3] = a;
	#else
		#error "No framebuffer type configured."
	#endif
}

uint32_t driver_framebuffer_getPixel(Frame* frame, int16_t x, int16_t y)
{
	uint8_t* buffer; int16_t width, height;
	if (!_getFrameContext(frame, &buffer, &width, &height)) return 0;
	if (!driver_framebuffer_orientation_apply(frame ? frame->window : NULL, &x, &y)) return 0;

	#if defined(FB_TYPE_1BPP)
		#ifndef FB_1BPP_VERT
			uint32_t position = (y * width) + (x / 8);
			uint8_t  bit      = x % 8;
		#else
			uint32_t position = ( (y / 8) * width) + x;
			uint8_t  bit      = y % 8;
		#endif
		if ((buffer[position] >> bit) && 0x01) {
			return 0xFFFFFF;
		} else {
			return 0x000000;
		}
	#elif defined(FB_TYPE_8BPP)
		uint32_t position = (y * width) + x;
		return (buffer[position] << 16) + (buffer[position]<<8) + buffer[position];
	#elif defined(FB_TYPE_16BPP)
		uint32_t position = (y * width * 2) + (x * 2);
		uint32_t color = (buffer[position] << 8) + (buffer[position + 1]);
		uint8_t r = ((((color >> 11) & 0x1F) * 527) + 23) >> 6;
		uint8_t g = ((((color >> 5 ) & 0x3F) * 259) + 33) >> 6;
		uint8_t b = ((((color      ) & 0x1F) * 527) + 23) >> 6;
		return r << 16 | g << 8 | b;
	#elif defined(FB_TYPE_24BPP)
		uint32_t position = (y * width * 3) + (x * 3);
		return (buffer[position+2] << 16) + (buffer[position+1] << 8) + (buffer[position + 0]);
	#elif defined(FB_TYPE_32BPP)
		uint32_t position = (y * width * 4) + (x * 4);
		return (buffer[position+2] << 16) + (buffer[position+1] << 8) + (buffer[position + 0]);
	#else
		#error "No framebuffer type configured."
	#endif
}

bool driver_framebuffer_flush(uint32_t flags)
{
	if (!framebuffer) {
		ESP_LOGE(TAG, "flush without alloc!");
		return false;
	}
	
	Window* currentWindow = driver_framebuffer_first_window();
	while (currentWindow != NULL) {
		/*printf("Window: %s\n", currentWindow->name);
		if (currentWindow->_prevWindow) {
			printf(" - Previous: %s\n", currentWindow->_prevWindow->name);
		} else {
			printf(" - Previous: none\n");
		}
		if (currentWindow->_nextWindow) {
			printf(" - Next: %s\n", currentWindow->_nextWindow->name);
		} else {
			printf(" - Next: none\n");
		}*/
		if (currentWindow->visible) {
			Frame* currentFrame = currentWindow->frames; //Fixme.
			for (uint16_t wy = 0; wy < currentWindow->height; wy++) {
				for (uint16_t wx = 0; wx < currentWindow->width; wx++) {
					driver_framebuffer_setPixel(
						NULL,
						currentWindow->x + wx,
						currentWindow->y + wy,
						driver_framebuffer_getPixel(
							currentFrame,
							wx,
							wy
						)
					);
				}
			}
		}
		currentWindow = currentWindow->_nextWindow;
	}

	uint8_t eink_flags = 0;
	
	if ((flags & FB_FLAG_FULL) || (flags & FB_FLAG_FORCE)) {
		driver_framebuffer_set_dirty_area(0, 0, FB_WIDTH-1, FB_HEIGHT-1, true);
		#ifdef DISPLAY_FLAG_LUT_BIT
			eink_flags |= DRIVER_EINK_LUT_FULL << DISPLAY_FLAG_LUT_BIT;
		#endif
	} else if (!driver_framebuffer_is_dirty()) {
		return false; //No need to update, stop.
	}
	
	#ifdef CONFIG_DRIVER_HUB75_ENABLE
	//compositor_disable();
	#endif
	
	#if defined(FB_TYPE_8BPP) && defined(DISPLAY_FLAG_8BITPIXEL)
		eink_flags |= DISPLAY_FLAG_8BITPIXEL;
	#endif

	#ifdef DISPLAY_FLAG_LUT_BIT
		if (flags & FB_FLAG_LUT_NORMAL) eink_flags |= DRIVER_EINK_LUT_NORMAL << DISPLAY_FLAG_LUT_BIT;
		if (flags & FB_FLAG_LUT_FAST) eink_flags |= DRIVER_EINK_LUT_FASTER << DISPLAY_FLAG_LUT_BIT;
		if (flags & FB_FLAG_LUT_FASTEST) eink_flags |= DRIVER_EINK_LUT_FASTEST << DISPLAY_FLAG_LUT_BIT;
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

esp_err_t driver_framebuffer_png(Frame* frame, int16_t x, int16_t y, lib_reader_read_t reader, void* reader_p)
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
	//int bit_depth = pr->ihdr.bit_depth;
	//int color_type = pr->ihdr.color_type;
	
	driver_framebuffer_set_dirty_area(x, y, x + width - 1, y + height - 1, false);
	
	uint32_t dst_min_x = x < 0 ? -x : 0;
	uint32_t dst_min_y = y < 0 ? -y : 0;
			
	res = lib_png_load_image(pr, x, y, dst_min_x, dst_min_y, FB_WIDTH - x, FB_HEIGHT - y, FB_WIDTH);

	lib_png_destroy(pr);

	if (res < 0) {
		printf("Failed to load image.\n");
		return ESP_FAIL;
	}
	
	return ESP_OK;
}

uint16_t driver_framebuffer_getWidth(Window* window)
{
	int16_t width, height;
	driver_framebuffer_window_getSize(window, &width, &height);
	Frame* frame = NULL; if (window) frame = window->frames; //Temporary workaround
	driver_framebuffer_orientation_revert(frame ? frame->window : NULL, &width, &height);
	return width;
}

uint16_t driver_framebuffer_getHeight(Window* window)
{
	int16_t width, height;
	driver_framebuffer_window_getSize(window, &width, &height);
	Frame* frame = NULL; if (window) frame = window->frames; //Temporary workaround
	driver_framebuffer_orientation_revert(frame ? frame->window : NULL, &width, &height);
	return height;
}

#else
esp_err_t driver_framebuffer_init() { return ESP_OK; }
#endif
