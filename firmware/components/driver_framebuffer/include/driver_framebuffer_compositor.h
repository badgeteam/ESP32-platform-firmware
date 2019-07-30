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
	
	/* Identifier */
	uint16_t id;
	
	/* Placement */
	int16_t x, y;                   // Position (x,y)
	uint16_t width, height;         // Size
	bool visible;                   // Visible or hidden
	
	/* Transparency */
	uint8_t tpEnabled;              // Enable transparency (0 = completely visible, 255 = completely transparent)
	uint8_t tpValue;                // Which value (on or off) is transparent
	
	/* Movement */
	int16_t moveX, moveY;           // Amount of pixels to move on each step
	uint8_t moveDiv, _currMoveDiv;  // Move every X steps
	bool moveLoop;                  // Warp to the other side of the screen when moving ouf of bounds

	/* Frames */
	Frame* firstFrame;       // Starting point of linked list of frames
	
	/* State */
	enum Orientation orientation;   // Current orientation
	
	/* Animation */
	uint8_t aniDiv, _currAniDiv;    // Go to next frame every X steps
	bool aniLoop;                   // Loop animation
	
	/* Text */
	const GFXfont *font;            // Text drawing font
	uint8_t textScaleX;             // Horizontal text scale
	uint8_t textScaleY;             // Vertical text scale
	int16_t textCursorX;            // Current horizontal position
	int16_t textCursorX0;           // Starting position after newline
	int16_t textCursorY;            // Current vertical position
	bool textWrap;                  // Wrap text when reaching border of buffer
	uint32_t textColor;             // Text drawing color
} Window;

#endif
