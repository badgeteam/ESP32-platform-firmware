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
	#else
		frame->buffer                       = heap_caps_malloc((window->width*window->height*PIXEL_SIZE)/8, MALLOC_CAP_8BIT);
	#endif
	frame->window = window;
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
	Frame* newFrame = _create_frame(window);
	if (!newFrame) return NULL;
	
	newFrame->_prevFrame = window->_lastFrame; //Add pointer to previous last frame
	if (window->lastFrame) {
		//Add ourselves as next frame if there already is a frame
		window->_lastFrame->_nextFrame = newFrame;
	} else {
		//We are the first frame added to the window, add ourselves as the first frame and the last frame
		window->_lastFrame = newFrame;
		window->_firstFrame = newFrame;
	}
	window->frame = newFrame; //Switch the window to the new frame
	return newFrame;
}

void driver_framebuffer_window_set_frame(Window* window, Frame* frame)
{
	if (!window) return NULL;
	window->currentFrame = frame;
}

bool driver_framebuffer_window_next_frame(Window* window)
{
	if (!window) return NULL;
	if (window->frame->_nextFrame) {
		//There is a frame after the current frame
		window->frame = window->frame->_nextFrame;
	} else {
		//At the last frame...
		if (window->loopFrames) {
			//Loop back to the first frame
			window->frame = window->_firstFrame;
		} else {
			//Last frame already reached, report failure
			return false;
		}
	}
	return true;
}

bool driver_framebuffer_window_prev_frame(Window* window)
{
	if (!window) return NULL;
	if (window->frame->_prevFrame) {
		//There is a frame before the current frame
		window->frame = window->frame->_prevFrame;
	} else {
		//At the first frame...
		if (window->loopFrames) {
			//Loop back to the last frame
			window->frame = window->_lastFrame;
		} else {
			//First frame already reached, report failure
			return false;
		}
	}
	return true;
}

bool driver_framebuffer_window_seek_frame(Window* window, uint16_t nr)
{
	if (!window) return NULL;
	Frame* targetFrame = window->_firstFrame;
	for (uint16_t i = 0; i < nr; i++) {
		if (targetFrame->_nextFrame) {
			targetFrame = targetFrame->_nextFrame;
		} else {
			return false;
		}
	}
	window->frame = targetFrame;
	return true;
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
	Frame* frame = window->frames;
	window->frames = NULL;
	
	while (frame) {
		Frame* nextFrame = frame->_nextFrame;
		if (frame->buffer) free(frame->buffer);
		free(frame);
		frame = nextFrame;
	}
}

#endif /* CONFIG_DRIVER_FRAMEBUFFER_ENABLE */
