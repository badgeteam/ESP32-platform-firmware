/*
 * The functions in this file allow for managing frames
 * frames are small buffers that represent the contents
 * of a window during one animation step
 */

#include "include/driver_framebuffer_internal.h"

#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE

/* Public functions */

Frame* driver_framebuffer_frame_create(Window* window)
{
	Frame* frame = malloc(sizeof(Frame));
	if (!frame) return NULL;
	memset(frame, 0, sizeof(Frame));
	#ifdef CONFIG_DRIVER_FRAMEBUFFER_SPIRAM
		frame->buffer                       = heap_caps_malloc(((window->width*window->height*PIXEL_SIZE)/8)+1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
	#else
		frame->buffer                       = heap_caps_malloc(((window->width*window->height*PIXEL_SIZE)/8)+1, MALLOC_CAP_8BIT);
	#endif
	frame->window = window;
	if (!frame->buffer) {
		free(frame);
		return NULL;
	}
	driver_framebuffer_fill(frame, COLOR_FILL_DEFAULT);
	return frame;
}

void driver_framebuffer_compositor_frame_remove(Frame* frame)
{
	if (frame == NULL) return;
	Frame* nextFrame = frame->_nextFrame;
	Frame* prevFrame = frame->_prevFrame;
	if (prevFrame) prevFrame->_nextFrame = nextFrame; //Link previous to next
	if (nextFrame) nextFrame->_prevFrame = prevFrame; //Link next to previous
	if (frame->buffer) free(frame->buffer);
	free(frame);
}

#endif /* CONFIG_DRIVER_FRAMEBUFFER_ENABLE */
