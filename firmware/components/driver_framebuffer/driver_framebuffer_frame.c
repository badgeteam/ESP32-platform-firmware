/*
 * The functions in this file allow for managing frames
 * frames are small buffers that represent the contents
 * of a window during one animation step
 */

#include "include/driver_framebuffer_internal.h"

#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE

/* Variables */

/* Private functions */
inline Frame* _create_frame(Window* window)
{
	Frame* frame = malloc(sizeof(Frame));
	if (!frame) return NULL;
	memset(frame, 0, sizeof(Frame));
	#ifdef CONFIG_DRIVER_FRAMEBUFFER_SPIRAM
		frame->buffer                       = heap_caps_malloc((window->width*window->height*PIXEL_SIZE)/8, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
		#ifdef FB_ALPHA_ENABLED
			if (frame->buffer) frame->alpha = heap_caps_malloc(window->width*window->height, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
		#endif
	#else
		frame->buffer                       = heap_caps_malloc((window->width*window->height*PIXEL_SIZE)/8, MALLOC_CAP_8BIT);
	#endif
	if (!frame->buffer) {
		free(frame);
		return NULL;
	}
	return frame;
}

/* Public functions */

Frame* driver_framebuffer_add_frame_to_window(Window* window)
{
	if (!window) return NULL;
	Frame* lastFrame = window->firstFrame;
	while (lastFrame && lastFrame->_nextFrame) lastFrame = lastFrame->_nextFrame;
	Frame* newFrame = _create_frame(window);
	if (!newFrame) return NULL;
	newFrame->_prevFrame = lastFrame;
	if (lastFrame) lastFrame->_nextFrame = newFrame;
	return newFrame;
}

void driver_framebuffer_compositor_delete_frame(Frame* frame)
{
	if (frame == NULL) return;
	Frame* nextFrame = frame->_nextFrame;
	Frame* prevFrame = frame->_prevFrame;
	if (prevFrame) prevFrame->_nextFrame = nextFrame; //Link previous to next
	if (nextFrame) nextFrame->_prevFrame = prevFrame; //Link next to previous
	if (frame->buffer) free(frame->buffer);
	free(frame);
}

void driver_framebuffer_remove_all_frames_from_window(Window* window)
{
	if (!window) return;
	Frame* frame = window->firstFrame;
	window->firstFrame = NULL;
	
	while (frame) {
		Frame* nextFrame = frame->_nextFrame;
		if (frame->buffer) free(frame->buffer);
		free(frame);
		frame = nextFrame;
	}
}

#endif /* CONFIG_DRIVER_FRAMEBUFFER_ENABLE */
