#include "include/driver_framebuffer_compositor.h"
#include "include/driver_framebuffer_internal.h"

#define TAG "fb-compositor"
#define DEFAULT_FONT &freesans9pt7b

Window* windows = NULL;    //Linked list of windows
uint16_t nextWindowId = 0; //Identifier of the next window that will be created

Window* driver_framebuffer_compositor_create_window(int16_t x, int16_t y, uint16_t width, uint16_t height)
{
	Window* window = malloc(sizeof(Window));
	if (!window) return NULL;
	memset(window, 0, sizeof(Window));
	window->id = nextWindowId;
	window->x = x;
	window->y = y;
	window->width = width;
	window->height = height;
	window->font = DEFAULT_FONT;
	window->textScaleX = 1;
	window->textScaleY = 1;
	window->textWrap = true;	
	nextWindowId++;
	return window;
}

void driver_framebuffer_compositor_delete_window(Window* window)
{
	if (window == NULL) return;
	Window* prevWindow = window->_prevWindow;
	Window* nextWindow = window->_nextWindow;
	if (prevWindow) prevWindow->_nextWindow = nextWindow; //Link previous to next
	if (nextWindow) nextWindow->_prevWindow = prevWindow; //Link next to previous
	free(window);
}
