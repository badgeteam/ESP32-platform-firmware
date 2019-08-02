#ifndef _DRIVER_FRAMEBUFFER_COMPOSITOR_H_
#define _DRIVER_FRAMEBUFFER_COMPOSITOR_H_
#include "sdkconfig.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "esp_system.h"

#include "driver_framebuffer_orientation_internal.h"
#include "driver_framebuffer_font.h"

typedef struct Frame_t {
	/* Linked list */
	struct Frame_t* _prevFrame;
	struct Frame_t* _nextFrame;
	
	/* Parent */
	struct Window_t* window;
	
	/* This frame */
	uint8_t* buffer;
} Frame;

typedef struct Window_t {
	/* Linked list */
	struct Window_t* _prevWindow;
	struct Window_t* _nextWindow;
	
	/* Properties */
	char* name;                     //The name of the window
	int16_t x, y;                   // Position (x,y)
	uint16_t width, height;         // Size
	bool visible;                   // Visible or hidden
	uint8_t tpEnabled;              // Enable transparency (0 = completely visible, 255 = completely transparent)
	uint32_t tpValue;               // Which value (on or off) is transparent
	enum Orientation orientation;   // Current orientation
	int16_t textCursorX;            // Current horizontal position
	int16_t textCursorX0;           // Starting position after newline
	int16_t textCursorY;            // Current vertical position
	
	/* Frames */
	Frame* frames;                  // Starting point of linked list of frames
} Window;

Frame* driver_framebuffer_add_frame_to_window(Window* window);
void driver_framebuffer_compositor_delete_frame(Frame* frame);
void driver_framebuffer_remove_all_frames_from_window(Window* window);

Window* driver_framebuffer_create_window(const char* name, uint16_t width, uint16_t height);
/* Create a window */

void driver_framebuffer_remove_window(Window* window);
/* Delete a window */

Window* driver_framebuffer_find_window(const char* name);
/* Find an existing window by name */

Window* driver_framebuffer_first_window();
/* Find the first window */

Window* driver_framebuffer_last_window();
/* Find the last window */

void driver_framebuffer_focus_window(Window* window);
/* Move a window to the end of the list */

void driver_framebuffer_window_getSize(Window* window, int16_t* width, int16_t* height);
/* Get the width and height of a window */

#endif
