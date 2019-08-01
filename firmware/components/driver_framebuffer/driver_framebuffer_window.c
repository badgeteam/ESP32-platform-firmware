/*
 * The functions in this file allow for managing windows
 * windows are collections of frames that can be rendered
 * to the primary framebuffer
 */

#include "include/driver_framebuffer_internal.h"

#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE

/* Variables */
Window* windows = NULL;

/* Private functions */
inline Window* _create_window()
{ //Create an empty window object
	Window* window = malloc(sizeof(Window));
	if (!window) return NULL;
	memset(window, 0, sizeof(Window));
	return window;
}

inline void _add_window(Window* window)
{ //Add a window to the linked list
	if (windows == NULL) {
		windows = window;
		return;
	}
	
	Window* lastWindow = windows;
	while (lastWindow->_nextWindow != NULL) {
		lastWindow = lastWindow->_nextWindow;
	}
	lastWindow->_nextWindow = window;
}

inline void _remove_window(Window* window)
{ //Remove a window from the linked list
	Window* prevWindow = window->_prevWindow;
	Window* nextWindow = window->_nextWindow;
	if (prevWindow) prevWindow->_nextWindow = nextWindow; //Link previous to next
	if (nextWindow) nextWindow->_prevWindow = prevWindow; //Link next to previous
	if (prevWindow == NULL) windows = window;
}

/* Public functions */

Window* driver_framebuffer_create_window(const char* name, uint16_t width, uint16_t height)
{
	Window* window = _create_window();
	window->name = strdup(name);
	window->width = width;
	window->height = height;
	window->frames = driver_framebuffer_add_frame_to_window(window); //Create first frame
	window->_prevWindow = driver_framebuffer_last_window();
	window->_nextWindow = NULL;
	_add_window(window); //Add window to linked list of windows
	return window;
}

void driver_framebuffer_delete_window(Window* window)
{
	_remove_window(window);
	if (windows == window) windows = NULL;
	free(window);
}

Window* driver_framebuffer_find_window(const char* name)
{
	Window* currentWindow = windows;
	while (currentWindow != NULL) {
		if ((strlen(name) == strlen(currentWindow->name)) &&
			(strcmp(currentWindow->name, name)==0)
		) {
			return currentWindow;
		}
		currentWindow = currentWindow->_nextWindow;
	}
	return NULL;
}

Window* driver_framebuffer_first_window()
{
	return windows;
}

Window* driver_framebuffer_last_window()
{
	Window* lastWindow = windows;
	while (lastWindow && lastWindow->_nextWindow != NULL) {
		lastWindow = lastWindow->_nextWindow;
	}
	return lastWindow;
}

void driver_framebuffer_focus_window(Window* window)
{
	_remove_window(window);
	if (windows == window) windows = window->_nextWindow;
	window->_prevWindow = driver_framebuffer_last_window();
	_add_window(window);
	window->_nextWindow = NULL;
}

void driver_framebuffer_window_getSize(Window* window, int16_t* width, int16_t* height)
{
	if (window == NULL) {
		//No window provided, use global context
		*width = FB_WIDTH;
		*height = FB_HEIGHT;
	} else {
		//Window provided, use window context
		*width  = window->width;
		*height = window->height;
	}
}

#endif /* CONFIG_DRIVER_FRAMEBUFFER_ENABLE */
