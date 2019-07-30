#ifndef _DRIVER_FRAMEBUFFER_ORIENTATION_H_
#define _DRIVER_FRAMEBUFFER_ORIENTATION_H_

#include <stdint.h>
#include <stdbool.h>

#include "driver_framebuffer_compositor.h"

enum Orientation driver_framebuffer_get_orientation(Frame *frame);
/* Get the orientation of the window that contains the provided frame */

void driver_framebuffer_set_orientation(Frame *frame, enum Orientation newOrientation);
/* Set the orientation of the window that contains the provided frame */

uint16_t driver_framebuffer_get_orientation_angle(Frame *frame);
/* Get the orientation of the window that contains the provided frame as angle */

void driver_framebuffer_set_orientation_angle(Frame *frame, uint16_t angle);
/* Set the orientation of the window that contains the provided frame as angle */

bool driver_framebuffer_orientation_apply(Frame *frame, int16_t *x, int16_t *y);
/* Apply the orientation of the window that contains the provided frame to the provided coordinates. (the provided coordinates are from the users perspective) */

void driver_framebuffer_orientation_revert(Frame *frame, int16_t *x, int16_t *y);
/* Revert the orientation of the window that contains the provided frame to the provided coordinates. (the provided coordinates are from the internal perspective) */

void driver_framebuffer_orientation_revert_square(Frame* frame, int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1);
/* Revert the orientation of the window that contains the provided frame to the provided square coordinates. */

#endif
