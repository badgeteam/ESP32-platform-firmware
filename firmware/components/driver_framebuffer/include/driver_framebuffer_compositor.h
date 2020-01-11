#pragma once

#include "sdkconfig.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "esp_system.h"
#include "driver_framebuffer_orientation_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Window_t {
	/* Linked list */
	struct Window_t* _prevWindow;
	struct Window_t* _nextWindow;
	
	/* Properties */
	char* name;                     //The name of the window
	int16_t x, y;                   // Position (x, y)
	uint16_t width, height;         // Buffer size (width, height)
	enum Orientation orientation;   // Current orientation
	bool enableTransparentColor;    // Enable transparency
	uint32_t transparentColor;      // Which color is made transparent
	
	/* Automatic rendering settings */
	bool visible;                   // Visible or hidden
	int16_t hOffset, vOffset;       // Drawing offset (x, y)
	uint16_t drawWidth, drawHeight; // Drawing size (width, height)
	
	/* Buffer */
	uint8_t* buffer;
} Window;

Window* driver_framebuffer_window_create(const char* name, uint16_t width, uint16_t height);
/* Create a window */

void driver_framebuffer_window_remove(Window* window);
/* Delete a window */

Window* driver_framebuffer_window_find(const char* name);
/* Find an existing window by name */

Window* driver_framebuffer_window_first();
/* Find the first window */

Window* driver_framebuffer_window_last();
/* Find the last window */

void driver_framebuffer_window_focus(Window* window);
/* Move a window to the end of the list */

void driver_framebuffer_window_getSize(Window* window, int16_t* width, int16_t* height);
/* Get the width and height of a window */

#ifdef __cplusplus
}
#endif
