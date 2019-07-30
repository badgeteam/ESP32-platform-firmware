/*
 * The functions in this file translate coordinates between
 * the point-of-view of the user and the point-of-view of the
 * buffer in memory when changing the orientation of the framebuffer
 * or the orientation of a compositor frame.
 * 
 * Additionally the functions in this file allow for setting and 
 * querying the orientation for both the global framebuffer and
 * for each of the compositor windows.
 */

#include "include/driver_framebuffer_internal.h"

#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE

#define TAG "fb-orientation"

/* Variables */
enum Orientation globalOrientation;

/* Private functions */
inline enum Orientation* _getOrientationContext(Frame* frame)
{
	if (frame == NULL) {
		//No frame provided, use global context
		return &globalOrientation;
	} else {
		//Frame provided, use frame context
		return &frame->window->orientation;
	}
}

inline void _getSizeContext(Frame* frame, uint16_t* width, uint16_t* height)
{
	if (frame == NULL) {
		//No frame provided, use global context
		*width       = FB_WIDTH;
		*height      = FB_HEIGHT;
	} else {
		//Frame provided, use frame context
		*width       = frame->window->width;
		*height      = frame->window->height;
	}
}

/* Public functions */

enum Orientation driver_framebuffer_get_orientation(Frame* frame)
{
	enum Orientation *orientation = _getOrientationContext(frame);
	return *orientation;
}

void driver_framebuffer_set_orientation(Frame* frame, enum Orientation newOrientation)
{
	enum Orientation *orientation = _getOrientationContext(frame);
	*orientation = newOrientation;
}

uint16_t driver_framebuffer_get_orientation_angle(Frame* frame)
{
	enum Orientation *orientation = _getOrientationContext(frame);
	switch(*orientation) {
		case portrait:
			return 90;
		case reverse_landscape:
			return 180;
		case reverse_portrait:
			return 270;
		default:
		case landscape:
			return 0;
	}
}

void driver_framebuffer_set_orientation_angle(Frame* frame, uint16_t angle)
{
	enum Orientation *orientation = _getOrientationContext(frame);
	switch(angle) {
		case 90:
			ESP_LOGE(TAG, "Orientation set to portrait");
			*orientation = portrait;
			break;
		case 180:
			ESP_LOGE(TAG, "Orientation set to reverse landscape");
			*orientation = reverse_landscape;
			break;
		case 270:
			ESP_LOGE(TAG, "Orientation set to reverse portrait");
			*orientation = reverse_portrait;
			break;
		default:
			ESP_LOGE(TAG, "Orientation set to landscape");
			*orientation = landscape;
			break;
	}
}

bool driver_framebuffer_orientation_apply(Frame* frame, int16_t* x, int16_t* y)
{
	enum Orientation *orientation = _getOrientationContext(frame);
	uint16_t width, height;
	_getSizeContext(frame, &width, &height);
	
	if (*orientation == portrait || *orientation == reverse_portrait) { //90 degrees
		int16_t t = *y;
		*y = *x;
		*x = (width-1)-t;
	}
	
	if (*orientation == reverse_landscape || *orientation == reverse_portrait) { //180 degrees
		*y = (height-1)-*y;
		*x = (width-1)-*x;
	}
	return (*x >= 0) && (*x < width) && (*y >= 0) && (*y < height);
}

void driver_framebuffer_orientation_revert(Frame* frame, int16_t* x, int16_t* y)
{
	enum Orientation *orientation = _getOrientationContext(frame);
	uint16_t width, height;
	_getSizeContext(frame, &width, &height);

	printf("Orientation revert %u, %u\n", *x, *y);

	if (*orientation == reverse_landscape || *orientation == reverse_portrait) { //90 degrees
		int16_t t = *x;
		*x = *y;
		*y = (width-1)-t;
	}

	if (*orientation == portrait || *orientation == reverse_portrait) { //180 degrees
		*y = (height-1)-*y;
		*x = (width-1)-*x;
	}
}

void driver_framebuffer_orientation_revert_square(Frame* frame, int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1)
{
	enum Orientation *orientation = _getOrientationContext(frame);
	uint16_t width, height;
	_getSizeContext(frame, &width, &height);

	if (*orientation == reverse_landscape || *orientation == reverse_portrait) { //90 degrees
		int16_t tx0 = *x0;
		int16_t ty0 = *y0;
		*x0 = width-*x1-1;
		*y0 = height-*y1-1;
		*x1 = width-tx0-1;
		*y1 = height-ty0-1;
	}

	if (*orientation == portrait || *orientation == reverse_portrait) { //180 degrees
		int16_t tx0 = *x0;
		int16_t tx1 = *x1;
		int16_t ty1 = *y1;
		*x0 = *y0;
		*y0 = width-tx1-1;
		*x1 = ty1;
		*y1 = width-tx0-1;
	}
}

#endif /* CONFIG_DRIVER_FRAMEBUFFER_ENABLE */
