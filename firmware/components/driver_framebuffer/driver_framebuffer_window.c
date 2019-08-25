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

void _debug_windows()
{
	Window* window = windows;
	printf("Windows: ");
	while (window != NULL) {
		printf("%s ", window->name);
		window = window->_nextWindow;
	}
	printf("\n");
}

inline void _remove_window_from_linked_list(Window* window)
{ //Remove a window from the linked list
	Window* prevWindow = window->_prevWindow;
	Window* nextWindow = window->_nextWindow;
	if (prevWindow) prevWindow->_nextWindow = nextWindow; //Link previous to next
	if (nextWindow) nextWindow->_prevWindow = prevWindow; //Link next to previous
	if (windows == window) windows = nextWindow; //Change the start window of the linked list
}

/* Public functions */

Window* driver_framebuffer_window_create(const char* name, uint16_t width, uint16_t height)
{
	if (driver_framebuffer_window_find(name)) return NULL; //If the window already exists do nothing and return.
	Window* window = _create_window();
	window->name = strdup(name);
	window->width = width;
	window->height = height;
	window->frame = driver_framebuffer_window_add_frame(window); //Create first frame
	window->_prevWindow = driver_framebuffer_window_last();
	window->_nextWindow = NULL;
	window->transparentColor = 0xFF00FF;
	_add_window(window); //Add window to linked list of windows
	return window;
}

void driver_framebuffer_window_remove(Window* window)
{
	driver_framebuffer_window_remove_all_frames(window);
	_remove_window_from_linked_list(window);
	free(window);
}

Window* driver_framebuffer_window_find(const char* name)
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

Window* driver_framebuffer_window_first()
{
	return windows;
}

Window* driver_framebuffer_window_last()
{
	Window* lastWindow = windows;
	while (lastWindow && lastWindow->_nextWindow != NULL) {
		lastWindow = lastWindow->_nextWindow;
	}
	return lastWindow;
}

void driver_framebuffer_window_focus(Window* window)
{
	_remove_window_from_linked_list(window);
	if (windows == window) windows = window->_nextWindow;
	window->_prevWindow = driver_framebuffer_window_last();
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

Frame* driver_framebuffer_window_add_frame(Window* window)
{
	if (!window) return NULL;
	Frame* newFrame = driver_framebuffer_frame_create(window);
	if (!newFrame) return NULL;
	
	newFrame->_prevFrame = window->_lastFrame; //Add pointer to previous last frame
	if (window->_lastFrame) {
		//Add ourselves as next frame if there already is a frame
		window->_lastFrame->_nextFrame = newFrame;
	} else {
		//We are the first frame added to the window, add ourselves as the first frame and the last frame
		window->_lastFrame = newFrame;
		window->_firstFrame = newFrame;
	}
	return newFrame;
}

void driver_framebuffer_window_set_frame(Window* window, Frame* frame)
{
	if ((!window) || (!frame)) return;
	window->frame = frame;
}

Frame* driver_framebuffer_window_next_frame(Window* window)
{
	if (!window) return NULL;
	if (window->frame->_nextFrame) {
		//There is a frame after the current frame
		return window->frame->_nextFrame;
	} else {
		//At the last frame...
		if (window->loopFrames) {
			//Loop back to the first frame
			return window->_firstFrame;
		} else {
			//Last frame already reached
			return NULL;
		}
	}
}

Frame* driver_framebuffer_window_prev_frame(Window* window)
{
	if (!window) return NULL;
	if (window->frame->_prevFrame) {
		//There is a frame before the current frame
		return window->frame->_prevFrame;
	} else {
		//At the first frame...
		if (window->loopFrames) {
			//Loop back to the last frame
			return window->_lastFrame;
		} else {
			//First frame already reached
			return NULL;
		}
	}
	return true;
}

Frame* driver_framebuffer_window_seek_frame(Window* window, uint16_t nr)
{
	if (!window) return NULL;
	Frame* targetFrame = window->_firstFrame;
	for (uint16_t i = 0; i < nr; i++) {
		if (!targetFrame->_nextFrame) return NULL;
		targetFrame = targetFrame->_nextFrame;
	}
	return targetFrame;
}

void driver_framebuffer_window_remove_all_frames(Window* window)
{
	if (!window) return;
	Frame* frame = window->frame;
	window->frame = NULL;
	
	while (frame) {
		Frame* nextFrame = frame->_nextFrame;
		if (frame->buffer) free(frame->buffer);
		free(frame);
		frame = nextFrame;
	}
}

#endif /* CONFIG_DRIVER_FRAMEBUFFER_ENABLE */
