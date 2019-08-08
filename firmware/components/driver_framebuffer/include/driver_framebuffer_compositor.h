#ifndef _DRIVER_FRAMEBUFFER_COMPOSITOR_H_
#define _DRIVER_FRAMEBUFFER_COMPOSITOR_H_
#include "sdkconfig.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "esp_system.h"

#include "driver_framebuffer_orientation_internal.h"
//#include "driver_framebuffer_font.h"

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
	enum Orientation orientation;   // Current orientation
	
	/* Frames */
	Frame* _firstFrame;             // Starting point of linked list of frames
	Frame* _lastFrame;              // The last frame in the linked list of frames
	Frame* frame;                   // The currently selected frame
	bool   loopFrames;              // Go to the first frame when reaching the last frame and vice versa
	
	/* Transparency and alpha bending */
	bool enableTransparentColor;    // Enable transparency
	uint32_t transparentColor;      // Which color is made transparent
} Window;

/* --- FRAMES --- */

Frame* driver_framebuffer_frame_create(Window* window);
/* Create a frame */

Frame* driver_framebuffer_frame_add_after(Frame* frame);
/* Add a frame after an existing frame */

Frame* driver_framebuffer_frame_add_before(Frame* frame);
/* Add a frame before an existing frame */

void driver_framebuffer_compositor_frame_remove(Frame* frame);
/* Remove a frame */

/* --- WINDOWS --- */

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

Frame* driver_framebuffer_window_add_frame(Window* window);
/* Add a frame to a window */

void driver_framebuffer_window_set_frame(Window* window, Frame* frame);
/* Set the current frame of a window */

Frame* driver_framebuffer_window_next_frame(Window* window);
/* Get the next frame of a window (or NULL if no next frame is available) */

Frame* driver_framebuffer_window_prev_frame(Window* window);
/* Get the previous frame of a window (or NULL if no previous frame is available) */

Frame* driver_framebuffer_window_seek_frame(Window* window, uint16_t nr);
/* Get a specific frame of a window (or NULL if the specific frame does not exist) */

void driver_framebuffer_window_remove_all_frames(Window* window);
/* Remove all frames from a window */

#endif
