#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "py/objarray.h"

#include "extmod/vfs.h"
#include "extmod/vfs_native.h"

#include <driver_framebuffer.h>
#include <driver_framebuffer_matrix.h>
#include <driver_framebuffer_compositor.h>
#include <driver_framebuffer_devices.h>

#include <math.h>

#include "sdkconfig.h"

#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE

const GFXfont * defaultFont = &roboto_12pt7b;
uint32_t defaultTextColor = COLOR_TEXT_DEFAULT;
uint32_t defaultFillColor = COLOR_FILL_DEFAULT;

static uint8_t *mp_obj_to_u8_ptr(mp_obj_t obj, size_t *len) {
  if (MP_OBJ_IS_TYPE(obj, &mp_type_bytes)) {
    return (uint8_t *)mp_obj_str_get_data(obj, len);
  } else if(MP_OBJ_IS_TYPE(obj, &mp_type_bytearray)) {
    mp_obj_array_t *array = MP_OBJ_TO_PTR(obj);
    *len = array->len;
    return array->items;
  }
  return NULL;
}

static mp_obj_t framebuffer_flush(mp_uint_t n_args, const mp_obj_t *args)
{
	uint32_t flags = 0;
	
	if (n_args > 0) {
		flags = mp_obj_get_int(args[0]);
	}
	
	driver_framebuffer_flush(flags);
	return mp_const_none;
}

static mp_obj_t framebuffer_size(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	
	if ((n_args > 0) && (MP_OBJ_IS_STR(args[0]))) {
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
	}
		
	int16_t width, height;
	driver_framebuffer_get_orientation_size(window, &width, &height);
	
	mp_obj_t tuple[2];
	tuple[0] = mp_obj_new_int(width);
	tuple[1] = mp_obj_new_int(height);
	return mp_obj_new_tuple(2, tuple);
}

static mp_obj_t framebuffer_width(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	
	if ((n_args > 0) && (MP_OBJ_IS_STR(args[0]))) {
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
	}
	
	return mp_obj_new_int(driver_framebuffer_getWidth(window));
}

static mp_obj_t framebuffer_height(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	
	if ((n_args > 0) && (MP_OBJ_IS_STR(args[0]))) {
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
	}
	
	return mp_obj_new_int(driver_framebuffer_getHeight(window));
}

static mp_obj_t framebuffer_orientation(mp_uint_t n_args, const mp_obj_t *args) {
	Window* window = NULL;
	
	if ((n_args > 0) && (!(MP_OBJ_IS_STR(args[0]) || MP_OBJ_IS_INT(args[0])))) {
		//First argument is not a string or integer, return error message.
		mp_raise_ValueError("Expected the first argument to be either the name of a window (string) or the orientation to set (integer).");
		return mp_const_none;
	}
	
	if ((n_args > 1) && (!MP_OBJ_IS_INT(args[1]))) {
		//Second argument is not an integer, return error message.
		mp_raise_ValueError("Expected the second argument to be the orientation to set (integer).");
		return mp_const_none;
	}
	
	if ((n_args > 0) && (MP_OBJ_IS_STR(args[0]))) {
		//First argument is a string: we're operating on a window
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
	}
	
	if ((n_args > 0) && (MP_OBJ_IS_INT(args[n_args-1]))) {
		//Set the orientation (last argument is an integer)
		driver_framebuffer_set_orientation_angle(window, mp_obj_get_int(args[n_args-1]));
		return mp_const_none;
	} else { //Get the orientation (no arguments or one argument which is a string)
		return mp_obj_new_int(driver_framebuffer_get_orientation_angle(window));
	}
}

static mp_obj_t framebuffer_draw_raw(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	
	if (n_args > 5) { //A window was provided
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
	}
		
	size_t  length;
	uint8_t* data   = mp_obj_to_u8_ptr(args[n_args-5], &length);
	
	if (data == NULL) {
		mp_raise_ValueError("Expected a bytes or bytearray object");
		return mp_const_none;
	}
	
	int16_t start_x = mp_obj_get_int(args[n_args-4]);
	int16_t start_y = mp_obj_get_int(args[n_args-3]);
	int16_t width   = mp_obj_get_int(args[n_args-2]);
	int16_t height  = mp_obj_get_int(args[n_args-1]);
	
	if (length != width*height*3) {
		mp_raise_ValueError("Expected 3 bytes per pixel (red, green and blue)");
		return mp_const_none;
	}
	
	for (int16_t pos_x = 0; pos_x < width; pos_x++) {
		for (int16_t pos_y = 0; pos_y < height; pos_y++) {
			uint32_t value = data[(pos_x + pos_y * width) * 3 + 2] + (data[(pos_x + pos_y * width) * 3 + 1] << 8) + (data[(pos_x + pos_y * width) * 3] << 16);
			driver_framebuffer_setPixel(window, start_x+pos_x, start_y+pos_y, value);
		}
	}
	return mp_const_none;
}

static mp_obj_t framebuffer_window_create(mp_uint_t n_args, const mp_obj_t *args)
{
	const char* name   = mp_obj_str_get_str(args[0]);
	int16_t     width  = mp_obj_get_int(args[1]);
	int16_t     height = mp_obj_get_int(args[2]);
	if (!driver_framebuffer_window_create(name, width, height)) {
		mp_raise_ValueError("A window with the provided name exists already!");
	}
	return mp_const_none;
}

static mp_obj_t framebuffer_window_remove(mp_uint_t n_args, const mp_obj_t *args)
{
	const char* name = mp_obj_str_get_str(args[0]);
	Window* window = driver_framebuffer_window_find(name);
	if (!window) {
		mp_raise_ValueError("Window not found");
		return mp_const_none;
	}
	driver_framebuffer_window_remove(window);
	return mp_const_none;
}

static mp_obj_t framebuffer_window_move(mp_uint_t n_args, const mp_obj_t *args)
{
	const char* name = mp_obj_str_get_str(args[0]);
	Window* window = driver_framebuffer_window_find(name);
	if (!window) {
		mp_raise_ValueError("Window not found");
		return mp_const_none;
	}
	
	window->x = mp_obj_get_int(args[1]);
	window->y = mp_obj_get_int(args[2]);
	
	return mp_const_none;
}

static mp_obj_t framebuffer_window_hide(mp_uint_t n_args, const mp_obj_t *args)
{
	const char* name = mp_obj_str_get_str(args[0]);
	Window* window = driver_framebuffer_window_find(name);
	if (!window) {
		mp_raise_ValueError("Window not found");
		return mp_const_none;
	}
	window->visible = false;
	return mp_const_none;
}

static mp_obj_t framebuffer_window_show(mp_uint_t n_args, const mp_obj_t *args)
{
	const char* name = mp_obj_str_get_str(args[0]);
	Window* window = driver_framebuffer_window_find(name);
	if (!window) {
		mp_raise_ValueError("Window not found");
		return mp_const_none;
	}
	window->visible = true;
	return mp_const_none;
}

static mp_obj_t framebuffer_window_visiblity(mp_uint_t n_args, const mp_obj_t *args)
{
	const char* name = mp_obj_str_get_str(args[0]);
	Window* window = driver_framebuffer_window_find(name);
	if (!window) {
		mp_raise_ValueError("Window not found");
		return mp_const_none;
	}
	window->visible = mp_obj_get_int(args[1]);
	return mp_const_none;
}

static mp_obj_t framebuffer_window_focus(mp_uint_t n_args, const mp_obj_t *args)
{
	const char* name = mp_obj_str_get_str(args[0]);
	Window* window = driver_framebuffer_window_find(name);
	if (!window) {
		mp_raise_ValueError("Window not found");
		return mp_const_none;
	}
	driver_framebuffer_window_focus(window);
	return mp_const_none;
}

static mp_obj_t framebuffer_window_resize(mp_uint_t n_args, const mp_obj_t *args)
{
	printf("FIXME\n");
	/*
	* Approach:
	*  - Create new window
	*  - Copy old window to new window
	*  - Delete old window
	*  - Rename new window to match old window
	*/
	return mp_const_none;
}

static mp_obj_t framebuffer_window_list(mp_uint_t n_args, const mp_obj_t *args)
{
	printf("FIXME\n");
	return mp_const_none;
}

//Fixme: add window rename function

static mp_obj_t framebuffer_window_transparency(mp_uint_t n_args, const mp_obj_t *args)
{
	const char* name = mp_obj_str_get_str(args[0]);
	Window* window = driver_framebuffer_window_find(name);
	if (!window) {
		mp_raise_ValueError("Window not found");
		return mp_const_none;
	}
	if (n_args > 1) {
		window->enableTransparentColor = mp_obj_get_int(args[1]);
		if (n_args > 2) {
			window->transparentColor = mp_obj_get_int(args[2]);
		}
		return mp_const_none;
	}
	return mp_obj_new_int(window->transparentColor); //Fixme!
}

static mp_obj_t framebuffer_get_pixel(mp_uint_t n_args, const mp_obj_t *args) {
	Window* window = NULL;
	matrix_stack_2d* stack;

	bool raw = false;
	int paramOffset = 0;

	if (n_args > 0 && MP_OBJ_IS_STR(args[0])) { //A window was provided
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
		stack = window->stack_2d;
		if (n_args > 3) {
			raw = mp_obj_get_int(args[3]) != 0;
		}
		paramOffset = 1;
	}
	else
	{
		stack = &stack_2d_global;
		if (n_args > 2) {
			raw = mp_obj_get_int(args[2]) != 0;
		}
	}
	
	float x = mp_obj_get_float(args[paramOffset]);
	float y = mp_obj_get_float(args[paramOffset + 1]);
	#ifdef CONFIG_G_MATRIX_ENABLE
	if (!raw) {
		matrix_2d_transform_point(stack->current, &x, &y);
	}
	#endif
	
	return mp_obj_new_int(driver_framebuffer_getPixel(window, x, y));
}

static mp_obj_t framebuffer_draw_pixel(mp_uint_t n_args, const mp_obj_t *args) {
	Window* window = NULL;
	matrix_stack_2d* stack;

	bool raw = false;
	int paramOffset = 0;

	if (n_args > 0 && MP_OBJ_IS_STR(args[0])) { //A window was provided
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
		stack = window->stack_2d;
		if (n_args > 4) {
			raw = mp_obj_get_int(args[4]) != 0;
		}
		paramOffset = 1;
	}
	else
	{
		stack = &stack_2d_global;
		if (n_args > 3) {
			raw = mp_obj_get_int(args[3]) != 0;
		}
	}
	
	float x = mp_obj_get_float(args[paramOffset]);
	float y = mp_obj_get_float(args[paramOffset + 1]);
	#ifdef CONFIG_G_MATRIX_ENABLE
	if (!raw) {
		matrix_2d_transform_point(stack->current, &x, &y);
	}
	#endif
	uint32_t color = mp_obj_get_int(args[paramOffset + 2]);
	
	driver_framebuffer_setPixel(window, (int) (x + 0.5), (int) (y + 0.5), color);
	return mp_const_none;
}

static mp_obj_t framebuffer_draw_fill(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	uint32_t color = defaultFillColor;
	
	if ((n_args > 0) && (!(MP_OBJ_IS_STR(args[0]) || MP_OBJ_IS_INT(args[0])))) {
		//First argument is not a string or integer, return error message.
		mp_raise_ValueError("Expected the first argument to be either the name of a window (string) or the color to fill with (integer).");
		return mp_const_none;
	}
	
	if ((n_args > 1) && (!MP_OBJ_IS_INT(args[n_args-1]))) {
		//Second argument is not an integer, return error message.
		mp_raise_ValueError("Expected the second argument to be the color to fill with (integer).");
		return mp_const_none;
	}
	
	if ((n_args > 0) && (MP_OBJ_IS_STR(args[0]))) {
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
	}
	
	if ((n_args > 0) && (MP_OBJ_IS_INT(args[n_args-1]))) {
		//Last argument is the color as an integer
		color = mp_obj_get_int(args[n_args-1]);
	}
	
	driver_framebuffer_fill(window, color);
	return mp_const_none;
}

static mp_obj_t framebuffer_draw_line(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	matrix_stack_2d* stack = NULL;
	if (MP_OBJ_IS_STR(args[0])) {
		if (n_args != 6) {
			mp_raise_ValueError("Expected 5 or 6 arguments: (window), x0, y0, x1, y1 and color");
			return mp_const_none;
		}
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
		stack = window->stack_2d;
	}
	else
	{
		stack = &stack_2d_global;
	}
	
	float x0 = mp_obj_get_float(args[n_args-5]);
	float y0 = mp_obj_get_float(args[n_args-4]);
	float x1 = mp_obj_get_float(args[n_args-3]);
	float y1 = mp_obj_get_float(args[n_args-2]);
	uint32_t color = mp_obj_get_int(args[n_args-1]);

	#ifdef CONFIG_G_MATRIX_ENABLE
	//transform point according to the transformation
	matrix_2d_transform_point(stack->current, &x0, &y0);
	matrix_2d_transform_point(stack->current, &x1, &y1);
	#endif
	//convert back to int so the line drawer will accept it
	int16_t x0i = (int16_t) (x0 + 0.5);
	int16_t y0i = (int16_t) (y0 + 0.5);
	int16_t x1i = (int16_t) (x1 + 0.5);
	int16_t y1i = (int16_t) (y1 + 0.5);

	driver_framebuffer_line(window, x0i, y0i, x1i, y1i, color);
	return mp_const_none;
}

#ifdef CONFIG_G_NEW_TRIANGLE
static mp_obj_t framebuffer_draw_triangle(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	matrix_stack_2d* stack;
	int paramOffset = 0;
	if (MP_OBJ_IS_STR(args[0])) {
		if (n_args != 7) {
			mp_raise_ValueError("Expected 6 or 7 arguments: (window), x0, y0, x1, y1, x2, y2, color");
			return mp_const_none;
		}
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
		paramOffset = 1;
		stack = window->stack_2d;
	}
	else
	{
		stack = &stack_2d_global;
	}
	
	float x0 = mp_obj_get_int(args[paramOffset]);
	float y0 = mp_obj_get_int(args[paramOffset + 1]);
	float x1 = mp_obj_get_int(args[paramOffset + 2]);
	float y1 = mp_obj_get_int(args[paramOffset + 3]);
	float x2 = mp_obj_get_int(args[paramOffset + 4]);
	float y2 = mp_obj_get_int(args[paramOffset + 5]);
	uint32_t color = mp_obj_get_int(args[paramOffset + 6]);
	matrix_2d_transform_point(stack->current, &x0, &y0);
	matrix_2d_transform_point(stack->current, &x1, &y1);
	matrix_2d_transform_point(stack->current, &x2, &y2);
	driver_framebuffer_triangle(window, x0, y0, x1, y1, x2, y2, color);
	return mp_const_none;
}
#endif

#ifdef CONFIG_G_NEW_QUAD
static mp_obj_t framebuffer_draw_quad(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	matrix_stack_2d* stack;
	int paramOffset = 0;
	if (MP_OBJ_IS_STR(args[0])) {
		if (n_args != 7) {
			mp_raise_ValueError("Expected 9 or 10 arguments: (window), x0, y0, x1, y1, x2, y2, x3, y3, color");
			return mp_const_none;
		}
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
		paramOffset = 1;
		stack = window->stack_2d;
	}
	else
	{
		stack = &stack_2d_global;
	}
	
	float x0 = mp_obj_get_int(args[paramOffset]);
	float y0 = mp_obj_get_int(args[paramOffset + 1]);
	float x1 = mp_obj_get_int(args[paramOffset + 2]);
	float y1 = mp_obj_get_int(args[paramOffset + 3]);
	float x2 = mp_obj_get_int(args[paramOffset + 4]);
	float y2 = mp_obj_get_int(args[paramOffset + 5]);
	float x3 = mp_obj_get_int(args[paramOffset + 6]);
	float y3 = mp_obj_get_int(args[paramOffset + 7]);
	uint32_t color = mp_obj_get_int(args[paramOffset + 8]);
	matrix_2d_transform_point(stack->current, &x0, &y0);
	matrix_2d_transform_point(stack->current, &x1, &y1);
	matrix_2d_transform_point(stack->current, &x2, &y2);
	matrix_2d_transform_point(stack->current, &x3, &y3);
	driver_framebuffer_quad(window, x0, y0, x1, y1, x2, y2, x3, y3, color);
	return mp_const_none;
}
#endif

static mp_obj_t framebuffer_draw_rect(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	matrix_stack_2d* stack;
	if (MP_OBJ_IS_STR(args[0])) {
		if (n_args != 7) {
			mp_raise_ValueError("Expected 6 or 7 arguments: (window), x, y, width, height, fill and color");
			return mp_const_none;
		}
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
		stack = window->stack_2d;
	}
	else
	{
		stack = &stack_2d_global;
	}

	#ifdef CONFIG_G_NEW_RECT
	float x0 = mp_obj_get_float(args[n_args-6]);
	float y0 = mp_obj_get_float(args[n_args-5]);
	float w = mp_obj_get_float(args[n_args-4]);
	float h = mp_obj_get_float(args[n_args-3]);
	float x1 = x0 + w;
	float y1 = y0;
	float x2 = x1;
	float y2 = y0 + h;
	float x3 = x0;
	float y3 = y2;
	matrix_2d_transform_point(stack->current, &x0, &y0);
	matrix_2d_transform_point(stack->current, &x1, &y1);
	matrix_2d_transform_point(stack->current, &x2, &y2);
	matrix_2d_transform_point(stack->current, &x3, &y3);
	int fill = mp_obj_get_int(args[n_args-2]);
	uint32_t color = mp_obj_get_int(args[n_args-1]);
	//driver_framebuffer_rect(window, x, y, w, h, fill, color);
	if (fill) {
		driver_framebuffer_quad(window, x0, y0, x1, y1, x2, y2, x3, y3, color);
	}
	else
	{
		driver_framebuffer_line(window, x0, y0, x1, y1, color);
		driver_framebuffer_line(window, x1, y1, x2, y2, color);
		driver_framebuffer_line(window, x2, y2, x3, y3, color);
		driver_framebuffer_line(window, x3, y3, x0, y0, color);
	}
	#else
	int x0 = mp_obj_get_int(args[n_args-6]);
	int y0 = mp_obj_get_int(args[n_args-5]);
	int w = mp_obj_get_int(args[n_args-4]);
	int h = mp_obj_get_int(args[n_args-3]);
	int fill = mp_obj_get_int(args[n_args-2]);
	uint32_t color = mp_obj_get_int(args[n_args-1]);
	driver_framebuffer_rect(window, x0, y0, w, h, fill, color);
	#endif
	return mp_const_none;
}

static mp_obj_t framebuffer_draw_circle(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	matrix_stack_2d* stack;
	if (MP_OBJ_IS_STR(args[0])) {
		if (n_args != 6) {
			mp_raise_ValueError("Expected 7 or 8 arguments: (window), x, y, radius, starting-angle, ending-angle, fill and color");
			return mp_const_none;
		}
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
		stack = window->stack_2d;
	}
	else
	{
		stack = &stack_2d_global;
	}
	
	#ifdef CONFIG_G_NEW_CIRCLE
	float x   = mp_obj_get_float(args[n_args-7]);
	float y   = mp_obj_get_float(args[n_args-6]);
	float r   = mp_obj_get_float(args[n_args-5]);
	float a0  = mp_obj_get_float(args[n_args-4]);
	float a1  = mp_obj_get_float(args[n_args-3]);
	int fill  = mp_obj_get_int(args[n_args-2]);
	uint32_t color = mp_obj_get_int(args[n_args-1]);
	driver_framebuffer_circle_new(window, stack, x, y, r, a0 * M_PI / 180.0, a1 * M_PI / 180.0, fill, color);
	#else
	int x   = mp_obj_get_int(args[n_args-7]);
	int y   = mp_obj_get_int(args[n_args-6]);
	int r   = mp_obj_get_int(args[n_args-5]);
	int a0  = mp_obj_get_int(args[n_args-4]);
	int a1  = mp_obj_get_int(args[n_args-3]);
	int fill  = mp_obj_get_int(args[n_args-2]);
	uint32_t color = mp_obj_get_int(args[n_args-1]);
	driver_framebuffer_circle(window, x, y, r, a0, a1, fill, color);
	#endif
	return mp_const_none;
}

static mp_obj_t framebuffer_draw_text(mp_uint_t n_args, const mp_obj_t *args) {
	uint8_t argOffset = 0;
	Window* window = NULL;
	if (MP_OBJ_IS_STR(args[0])) { //window, x, y, text ...
		if (n_args < 4) {
			mp_raise_ValueError("Expected window, x, y, text, ...");
			return mp_const_none;
		}
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[argOffset++]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
	}

	int x = mp_obj_get_int(args[argOffset++]);
	int y = mp_obj_get_int(args[argOffset++]);
	
	int textArg = argOffset++;
	
	uint32_t color = defaultTextColor;
	if (n_args>argOffset) color = mp_obj_get_int(args[argOffset++]);
	
	const GFXfont *font = defaultFont;
	if (n_args>argOffset) font = driver_framebuffer_findFontByName(mp_obj_str_get_str(args[argOffset++]));
	
	if (!font) {
		mp_raise_ValueError("Font not found");
		return mp_const_none;
	}
	
	uint16_t xScale = 1, yScale = 1;
	if (n_args>argOffset) xScale = mp_obj_get_int(args[argOffset++]);
	if (n_args>argOffset) yScale = mp_obj_get_int(args[argOffset++]);
	
	if (MP_OBJ_IS_STR(args[textArg])) {
		const char *text = mp_obj_str_get_str(args[textArg]);
		driver_framebuffer_print(window, text, x, y, xScale, yScale, color, font);
	} else {
		int chr = mp_obj_get_int(args[textArg]);
		char *text = malloc(2);
		text[0] = chr;
		text[1] = 0;
		driver_framebuffer_print(window, text, x, y, xScale, yScale, color, font);
		free(text);
	}
	return mp_const_none;
}

static mp_obj_t framebuffer_get_text_width(mp_uint_t n_args, const mp_obj_t *args) {
	const char *text = mp_obj_str_get_str(args[0]);
	const GFXfont *font = defaultFont;
	if (n_args>1) font = driver_framebuffer_findFontByName(mp_obj_str_get_str(args[1]));
	if (!font) {
		mp_raise_ValueError("Font not found");
		return mp_const_none;
	}
	int value = driver_framebuffer_get_string_width(text, font);
	return mp_obj_new_int(value);
}

static mp_obj_t framebuffer_get_text_height(mp_uint_t n_args, const mp_obj_t *args) {
	const char *text = mp_obj_str_get_str(args[0]);
	const GFXfont *font = defaultFont;
	if (n_args>1) font = driver_framebuffer_findFontByName(mp_obj_str_get_str(args[1]));
	if (!font) {
		mp_raise_ValueError("Font not found");
		return mp_const_none;
	}
	int value = driver_framebuffer_get_string_height(text, font);
	return mp_obj_new_int(value);
}

static mp_obj_t framebuffer_png_info(mp_uint_t n_args, const mp_obj_t *args)
{
	lib_reader_read_t reader;
	void * reader_p;
	bool is_bytes = MP_OBJ_IS_TYPE(args[0], &mp_type_bytes);
	if (is_bytes) {
		size_t len;
		const uint8_t* png_data = (const uint8_t *) mp_obj_str_get_data(args[0], &len);
		struct lib_mem_reader *mr = lib_mem_new(png_data, len);
		if (mr == NULL) {
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "out of memory!"));
			return mp_const_none;
		}
		reader = (lib_reader_read_t) &lib_mem_read;
		reader_p = mr;
	} else {
		const char* filename = mp_obj_str_get_str(args[0]);
		char fullname[128] = {'\0'};
		int res = physicalPathN(filename, fullname, sizeof(fullname));
		if ((res != 0) || (strlen(fullname) == 0)) {
			mp_raise_ValueError("Error resolving file name");
			return mp_const_none;
		}

		struct lib_file_reader *fr = lib_file_new(fullname, 1024);
		if (fr == NULL) {
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "Could not open file '%s'!",filename));
			return mp_const_none;
		}
		
		reader = (lib_reader_read_t) &lib_file_read;
		reader_p = fr;

	}

	struct lib_png_reader *pr = lib_png_new(reader, reader_p);
	if (pr == NULL) {
		if (is_bytes) {
			lib_mem_destroy(reader_p);
		} else {
			lib_file_destroy(reader_p);
		}

		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "out of memory."));
		return mp_const_none;
	}

	int res = lib_png_read_header(pr);
	mp_obj_t tuple[4];
	if (res >= 0) {
		tuple[0] = mp_obj_new_int(pr->ihdr.width);
		tuple[1] = mp_obj_new_int(pr->ihdr.height);
		tuple[2] = mp_obj_new_int(pr->ihdr.bit_depth);
		tuple[3] = mp_obj_new_int(pr->ihdr.color_type);
	}

	lib_png_destroy(pr);
	if (is_bytes) {
		lib_mem_destroy(reader_p);
	} else {
		lib_file_destroy(reader_p);
	}

	if (res < 0) {
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "failed to load image: res = %d", res));
	}
	return mp_obj_new_tuple(4, tuple);
}

static mp_obj_t framebuffer_draw_png(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	int paramOffset = 0;
	
	if (MP_OBJ_IS_STR(args[0])) {
		if (n_args < 4) {
			mp_raise_ValueError("Expected: window, x, y, file");
			return mp_const_none;
		}
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
		paramOffset++;
	}
	
	int16_t x = mp_obj_get_int(args[paramOffset++]);
	int16_t y = mp_obj_get_int(args[paramOffset++]);
	
	lib_reader_read_t reader;
	
	bool is_bytes = MP_OBJ_IS_TYPE(args[paramOffset], &mp_type_bytes);
	
	esp_err_t renderRes = ESP_FAIL;
	
	if (is_bytes) {
		mp_uint_t len;
		uint8_t *data = (uint8_t *)mp_obj_str_get_data(args[paramOffset], &len);
		struct lib_mem_reader *mr = lib_mem_new(data, len);
		if (mr == NULL) {
			mp_raise_ValueError("Out of memory");
			return mp_const_none;
		}
		reader = (lib_reader_read_t) &lib_mem_read;
		renderRes = driver_framebuffer_png(NULL, x, y, reader, mr);
		lib_mem_destroy(mr);
	} else {
		const char* filename = mp_obj_str_get_str(args[paramOffset]);
		char fullname[128] = {'\0'};
		int res = physicalPathN(filename, fullname, sizeof(fullname));
		if ((res != 0) || (strlen(fullname) == 0)) {
			mp_raise_ValueError("File not found");
			return mp_const_none;
		}
		struct lib_file_reader *fr = lib_file_new(fullname, 1024);
		if (fr == NULL) {
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "Could not open file '%s'!",filename));
			return mp_const_none;
		}
		reader = (lib_reader_read_t) &lib_file_read;
		renderRes = driver_framebuffer_png(window, x, y, reader, fr);
		lib_file_destroy(fr);
	}
	
	if (renderRes != ESP_OK) {
		mp_raise_ValueError("Rendering error");
	}
	
	return mp_const_none;
}

static mp_obj_t framebuffer_backlight(mp_uint_t n_args, const mp_obj_t *args)
{
	if (n_args > 0) {
		uint8_t brightness = mp_obj_get_int(args[0]);
		esp_err_t res = driver_framebuffer_setBacklight(brightness);
		if (res != ESP_OK) {
			mp_raise_ValueError("Failed to set backlight brightness!");
		}
		return mp_const_none;
	} else {
		return mp_obj_new_int(driver_framebuffer_getBacklight());
	}
}

extern const char* fontNames[];

static mp_obj_t framebuffer_list_fonts(mp_uint_t n_args, const mp_obj_t *args)
{
	uint16_t amount = 0;
	while (fontNames[amount] != NULL) amount++;
	
	mp_obj_t* tuple = calloc(amount, sizeof(mp_obj_t));
	if (!tuple) {
		mp_raise_ValueError("Out of memory");
		return mp_const_none;
	}
	for (uint16_t i = 0; i < amount; i++) {
		tuple[i] = mp_obj_new_str(fontNames[i], strlen(fontNames[i]));
	}
	
	return mp_obj_new_tuple(amount, tuple);
}

static mp_obj_t framebuffer_set_default_font(mp_uint_t n_args, const mp_obj_t *args)
{
	const GFXfont* font = driver_framebuffer_findFontByName(mp_obj_str_get_str(args[0]));
	if (!font) {
		mp_raise_ValueError("Font not found");
		return mp_const_none;
	}
	defaultFont = font;
	return mp_const_none;
}

static mp_obj_t framebuffer_default_text_color(mp_uint_t n_args, const mp_obj_t *args)
{
	if (n_args > 0) {
		defaultTextColor = mp_obj_get_int(args[0]);
		return mp_const_none;
	} else {
		return mp_obj_new_int(defaultTextColor);
	}
}

static mp_obj_t framebuffer_default_fill_color(mp_uint_t n_args, const mp_obj_t *args)
{
	if (n_args > 0) {
		defaultFillColor = mp_obj_get_int(args[0]);
		return mp_const_none;
	} else {
		return mp_obj_new_int(defaultFillColor);
	}
}



//Begin matrix stack
#ifdef CONFIG_G_MATRIX_ENABLE
static mp_obj_t framebuffer_pushMatrix(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	matrix_stack_2d* stack = NULL;
	int paramOffset = 0;
	
	if (n_args > 0 && MP_OBJ_IS_STR(args[0])) {
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
		paramOffset++;
		stack = window->stack_2d;
	}
	else
	{
		stack = &stack_2d_global;
	}
	
	esp_err_t resp = matrix_stack_2d_push(stack);
	if (resp != ESP_OK) {
		mp_raise_msg(&mp_type_Exception, "The matrix stack is full and cannot be pushed once more!");
	}
	return mp_const_none;
}

static mp_obj_t framebuffer_popMatrix(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	matrix_stack_2d* stack = NULL;
	int paramOffset = 0;
	
	if (n_args > 0 && MP_OBJ_IS_STR(args[0])) {
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
		paramOffset++;
		stack = window->stack_2d;
	}
	else
	{
		stack = &stack_2d_global;
	}
	
	esp_err_t resp = matrix_stack_2d_pop(stack);
	if (resp != ESP_OK) {
		mp_raise_msg(&mp_type_Exception, "The matrix stack is empty and cannot be popped once more!");
	}
	return mp_const_none;
}

static mp_obj_t framebuffer_clearMatrix(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	matrix_stack_2d* stack = NULL;
	int paramOffset = 0;
	bool keepStack = false;
	
	if (n_args > 0 && MP_OBJ_IS_STR(args[0])) {
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
		paramOffset++;
		stack = window->stack_2d;
		if (n_args > 1) {
			keepStack = mp_obj_is_true(args[1]);
		}
	}
	else
	{
		stack = &stack_2d_global;
		if (n_args > 0) {
			keepStack = mp_obj_is_true(args[0]);
		}
	}
	
	if (!keepStack) {
		matrix_stack_2d_clear(stack);
	}
	else
	{
		stack->current = matrix_2d_identity();
	}
	return mp_const_none;
}

static mp_obj_t framebuffer_matrixSize(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	matrix_stack_2d* stack = NULL;
	int paramOffset = 0;
	
	if (n_args > 0 && MP_OBJ_IS_STR(args[0])) {
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
		paramOffset++;
		stack = window->stack_2d;
	}
	else
	{
		stack = &stack_2d_global;
	}
	
	return mp_obj_new_int(stack->size);
}

static mp_obj_t framebuffer_getMatrix(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	matrix_stack_2d* stack = NULL;
	int paramOffset = 0;
	
	if (n_args > 0 && MP_OBJ_IS_STR(args[0])) {
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
		paramOffset++;
		stack = window->stack_2d;
	}
	else
	{
		stack = &stack_2d_global;
	}
	
	matrix_2d current = stack->current;
	mp_obj_t out[6] = {
		mp_obj_new_float(current.arr[0]),
		mp_obj_new_float(current.arr[1]),
		mp_obj_new_float(current.arr[2]),
		mp_obj_new_float(current.arr[3]),
		mp_obj_new_float(current.arr[4]),
		mp_obj_new_float(current.arr[5])
	};
	return mp_obj_new_list(6, out);
}

static mp_obj_t framebuffer_setMatrix(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	matrix_stack_2d* stack = NULL;
	int paramOffset = 0;
	
	if (MP_OBJ_IS_STR(args[0])) {
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
		paramOffset++;
		stack = window->stack_2d;
	}
	else
	{
		stack = &stack_2d_global;
	}
	
	if (MP_OBJ_IS_SMALL_INT(args[paramOffset]) || MP_OBJ_IS_QSTR(args[paramOffset]) || mp_obj_is_float(args[paramOffset])) {
		mp_raise_ValueError("Expected 6 enties in list, by index:\n[ 0  1  2 ]\n[ 3  4  5 ]");
	}

	mp_obj_t *list;
	size_t list_len;
	matrix_2d matrix;
	mp_obj_list_get(args[paramOffset], &list_len, &list);

	if (list_len != 6) {
		mp_raise_ValueError("Expected 6 enties in list, by index:\n[ 0  1  2 ]\n[ 3  4  5 ]");
		return mp_const_none;
	}

	for (int i = 0; i < 6; i++) {
		matrix.arr[i] = mp_obj_get_float(list[i]);
	}

	stack->current = matrix;

	return mp_const_none;
}

static mp_obj_t framebuffer_transformPoint(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	matrix_stack_2d* stack = NULL;
	int paramOffset = 0;
	
	if (MP_OBJ_IS_STR(args[0])) {
		if (n_args < 3) {
			mp_raise_ValueError("Expected: window, x, y");
			return mp_const_none;
		}
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
		paramOffset++;
		stack = window->stack_2d;
	}
	else
	{
		if (n_args < 2) {
			mp_raise_ValueError("Expected: x, y");
			return mp_const_none;
		}
		stack = &stack_2d_global;
	}
	
	float x = mp_obj_get_float(args[paramOffset]);
	float y = mp_obj_get_float(args[paramOffset + 1]);

	//printf("%f, %f -> ", x, y);
	matrix_2d_transform_point(stack->current, &x, &y);
	//printf("%f, %f\n", x, y);

	mp_obj_t out[2] = {
		mp_obj_new_float(x),
		mp_obj_new_float(y)
	};
	return mp_obj_new_tuple(2, out);
}

static mp_obj_t framebuffer_translate(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	matrix_stack_2d* stack = NULL;
	int paramOffset = 0;
	
	if (MP_OBJ_IS_STR(args[0])) {
		if (n_args < 3) {
			mp_raise_ValueError("Expected: window, x, y");
			return mp_const_none;
		}
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
		paramOffset++;
		stack = window->stack_2d;
	}
	else
	{
		if (n_args < 2) {
			mp_raise_ValueError("Expected: x, y");
			return mp_const_none;
		}
		stack = &stack_2d_global;
	}
	
	stack->current = matrix_2d_multiply(stack->current, matrix_2d_translate(mp_obj_get_float(args[paramOffset]), mp_obj_get_float(args[paramOffset + 1])));

	return mp_const_none;
}

static mp_obj_t framebuffer_rotate(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	matrix_stack_2d* stack = NULL;
	int paramOffset = 0;
	
	if (MP_OBJ_IS_STR(args[0])) {
		if (n_args < 2) {
			mp_raise_ValueError("Expected: window, angle");
			return mp_const_none;
		}
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
		paramOffset++;
		stack = window->stack_2d;
	}
	else
	{
		if (n_args < 1) {
			mp_raise_ValueError("Expected: angle");
			return mp_const_none;
		}
		stack = &stack_2d_global;
	}
	
	stack->current = matrix_2d_multiply(stack->current, matrix_2d_rotate(mp_obj_get_float(args[paramOffset])));

	return mp_const_none;
}

static mp_obj_t framebuffer_scale(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	matrix_stack_2d* stack = NULL;
	int paramOffset = 0;
	
	if (MP_OBJ_IS_STR(args[0])) {
		if (n_args < 3) {
			mp_raise_ValueError("Expected: window, xscale, yscale");
			return mp_const_none;
		}
		window = driver_framebuffer_window_find(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found");
			return mp_const_none;
		}
		paramOffset++;
		stack = window->stack_2d;
	}
	else
	{
		if (n_args < 2) {
			mp_raise_ValueError("Expected: xscale, yscale");
			return mp_const_none;
		}
		stack = &stack_2d_global;
	}
	
	stack->current = matrix_2d_multiply(stack->current, matrix_2d_scale(mp_obj_get_float(args[paramOffset]), mp_obj_get_float(args[paramOffset + 1])));

	return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_pushMatrix_obj,          0, 1, framebuffer_pushMatrix);
/* Arguments: window (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_popMatrix_obj,           0, 1, framebuffer_popMatrix);
/* Arguments: window (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_clearMatrix_obj,         0, 2, framebuffer_clearMatrix);
/* Arguments: window (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_matrixSize_obj,          0, 1, framebuffer_matrixSize);
/* Arguments: window (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_getMatrix_obj,           0, 1, framebuffer_getMatrix);
/* Arguments: window (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_setMatrix_obj,           1, 2, framebuffer_setMatrix);
/* Arguments: window (optional), matrix */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_transformPoint_obj,      1, 2, framebuffer_transformPoint);
/* Arguments: window (optional), x, y */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_translate_obj,           2, 3, framebuffer_translate);
/* Arguments: window (optional), x, y */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_rotate_obj,              1, 2, framebuffer_rotate);
/* Arguments: window (optional), angle */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_scale_obj,               2, 3, framebuffer_scale);
/* Arguments: window (optional), xscale, yscale */

#endif //CONFIG_G_MATRIX_ENABLE
//End matrix stack

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_flush_obj,                0, 1, framebuffer_flush);
/* Flush the framebuffer to the display. Arguments: flags (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_size_obj,                 0, 1, framebuffer_size);
/* Get the size (width, height) of the framebuffer or a window. Arguments: window (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_width_obj,                0, 1, framebuffer_width);
/* Get the width of the framebuffer or a window. Arguments: window (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_height_obj,               0, 1, framebuffer_height);
/* Get the height of the framebuffer or a window. Arguments: window (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_orientation_obj,          0, 2, framebuffer_orientation);
/* Get or set the orientation of the framebuffer or a window. Arguments: window (optional), orientation (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_draw_raw_obj,             5, 6, framebuffer_draw_raw);
/* Copy a raw bytes buffer directly to the framebuffer or a window. Arguments: window (optional), x, y, width, height, data */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_window_create_obj,        3, 3, framebuffer_window_create);
/* Create a new window. Arguments: window name, width, height */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_window_remove_obj,        1, 1, framebuffer_window_remove);
/* Delete a window. Arguments: window name */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_window_move_obj,          3, 3, framebuffer_window_move);
/* Move a window. Arguments: window name, x, y */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_window_hide_obj,          1, 1, framebuffer_window_hide);
/* Hide a window. Arguments: window name */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_window_show_obj,          1, 1, framebuffer_window_show);
/* Hide a window. Arguments: window name */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_window_visiblity_obj,     2, 2, framebuffer_window_visiblity);
/* Set the visibilty of a window. Arguments: window name, visible */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_window_focus_obj,         1, 1, framebuffer_window_focus);
/* Focus a window. Arguments: window name */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_window_resize_obj,        3, 3, framebuffer_window_resize);
/* Resize a window. Arguments: window name, width, height */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_window_list_obj,          0, 0, framebuffer_window_list);
/* Query a list of all window names */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_window_transparency_obj,  1, 3, framebuffer_window_transparency);
/* Query or configure transparency for a window. Arguments: window, enable (optional), color (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_get_pixel_obj,            2, 4, framebuffer_get_pixel);
/* Get the color of a pixel in the framebuffer or in a window. Arguments: window (optional), x, y, ignore transformation (optional, ignored without matrix stack) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_draw_pixel_obj,           3, 5, framebuffer_draw_pixel);
/* Set the color of a pixel in the framebuffer or in a window. Arguments: window (optional), x, y, color, ignore transformation (optional, ignored without matrix stack) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_draw_fill_obj,            0, 2, framebuffer_draw_fill);
/* Fill the framebuffer or a window with a color. Arguments: window (optional), color */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_draw_line_obj,            5, 6, framebuffer_draw_line);
/* Draw a line from point (x0,y0) to point (x1,y1) in the framebuffer or a window. Arguments: window (optional), x0, y0, x1, y1, color */

#ifdef CONFIG_G_NEW_QUAD
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_draw_quad_obj,            9,10, framebuffer_draw_quad);
/* Draw a rectangle in the framebuffer or a window. Arguments: window (optional), x0, y0, x1, y1, x2, y2, x3, y3, color*/
#endif

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_draw_rect_obj,            6, 7, framebuffer_draw_rect);
/* Draw a rectangle in the framebuffer or a window. Arguments: window (optional), x, y, width, height, color*/

#ifdef CONFIG_G_NEW_TRIANGLE
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_draw_triangle_obj,        7, 8, framebuffer_draw_triangle);
/* Draw a rectangle in the framebuffer or a window. Arguments: window (optional), x0, y0, x1, y1, x2, y2, color*/
#endif

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_draw_circle_obj,          7, 8, framebuffer_draw_circle);
/* Draw a circle in the framebuffer or a window. Arguments: window (optional), x, y, radius, starting-angle, ending-angle, fill, color */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_draw_text_obj,            3, 8, framebuffer_draw_text);
/* Draw text in the framebuffer or a window. Arguments: window (optional), x, y, text, color (optional), font (optional), x-scale (optional), y-scale (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_get_text_width_obj,       1, 2, framebuffer_get_text_width);
/* Get the width of a string when printed with a font. Arguments: text, font (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_get_text_height_obj,      1, 2, framebuffer_get_text_height);
/* Get the height of a string when printed with a font. Arguments: text, font (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_png_info_obj,              1, 1, framebuffer_png_info);
/* Get information about a PNG image. Arguments: buffer with PNG data or filename of PNG image */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_draw_png_obj,              3, 4, framebuffer_draw_png);
/* Draw a PNG image. Arguments: x, y, buffer with PNG data or filename of PNG image */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_backlight_obj,             0, 1, framebuffer_backlight);
/* Set or get the backlight brightness level. Arguments: level (0-255) (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_list_fonts_obj,            0, 0, framebuffer_list_fonts);
/* Query list of available fonts */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_set_default_font_obj,      1, 1, framebuffer_set_default_font);
/* Set default font */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_default_text_color_obj,    0, 1, framebuffer_default_text_color);
/* Set or get the default text color */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_default_fill_color_obj,    0, 1, framebuffer_default_fill_color);
/* Set or get the default fill color */

static const mp_rom_map_elem_t framebuffer_module_globals_table[] = {
	/* Constants */
	{MP_ROM_QSTR( MP_QSTR_FLAG_FORCE                    ), MP_ROM_INT( FB_FLAG_FORCE                         )}, //Refresh even when not dirty
	{MP_ROM_QSTR( MP_QSTR_FLAG_FULL                     ), MP_ROM_INT( FB_FLAG_FULL                          )}, //Full refresh (instead of partial refresh)
	{MP_ROM_QSTR( MP_QSTR_FLAG_LUT_GREYSCALE            ), MP_ROM_INT( FB_FLAG_LUT_GREYSCALE                 )}, //E-ink display: use greyscale LUT
	{MP_ROM_QSTR( MP_QSTR_FLAG_LUT_NORMAL               ), MP_ROM_INT( FB_FLAG_LUT_NORMAL                    )}, //E-ink display: use normal LUT
	{MP_ROM_QSTR( MP_QSTR_FLAG_LUT_FAST                 ), MP_ROM_INT( FB_FLAG_LUT_FAST                      )}, //E-ink display: use fast LUT
	{MP_ROM_QSTR( MP_QSTR_FLAG_LUT_FASTEST              ), MP_ROM_INT( FB_FLAG_LUT_FASTEST                   )}, //E-ink display: use fastest LUT
	
	{MP_ROM_QSTR( MP_QSTR_ORIENTATION_LANDSCAPE         ), MP_ROM_INT( 0                                     )}, //Orientation: landscape
	{MP_ROM_QSTR( MP_QSTR_ORIENTATION_PORTRAIT          ), MP_ROM_INT( 90                                    )}, //Orientation: portrait
	{MP_ROM_QSTR( MP_QSTR_ORIENTATION_REVERSE_LANDSCAPE ), MP_ROM_INT( 180                                   )}, //Orientation: reverse landscape
	{MP_ROM_QSTR( MP_QSTR_ORIENTATION_REVERSE_PORTRAIT  ), MP_ROM_INT( 270                                   )}, //Orientation: reverse portrait
	
	{MP_ROM_QSTR( MP_QSTR_WHITE                         ), MP_ROM_INT( 0xFFFFFF                              )}, //Color: white
	{MP_ROM_QSTR( MP_QSTR_BLACK                         ), MP_ROM_INT( 0x000000                              )}, //Color: black
	
	{MP_ROM_QSTR( MP_QSTR_RED                           ), MP_ROM_INT( 0xFF0000                              )}, //Color: red
	{MP_ROM_QSTR( MP_QSTR_GREEN                         ), MP_ROM_INT( 0x00FF00                              )}, //Color: green
	{MP_ROM_QSTR( MP_QSTR_BLUE                          ), MP_ROM_INT( 0x0000FF                              )}, //Color: blue
	
	{MP_ROM_QSTR( MP_QSTR_YELLOW                        ), MP_ROM_INT( 0xFFFF00                              )}, //Color: yellow
	{MP_ROM_QSTR( MP_QSTR_MAGENTA                       ), MP_ROM_INT( 0xFF00FF                              )}, //Color: magenta
	{MP_ROM_QSTR( MP_QSTR_CYAN                          ), MP_ROM_INT( 0x00FFFF                              )}, //Color: cyan
	
	/* Funcitons: color */
	{MP_ROM_QSTR( MP_QSTR_defaultFillColor              ), MP_ROM_PTR( &framebuffer_default_fill_color_obj   )}, //Set the default fill color
	{MP_ROM_QSTR( MP_QSTR_defaultTextColor              ), MP_ROM_PTR( &framebuffer_default_text_color_obj   )}, //Set the default text color
	
	/* Functions: hardware */
	{MP_ROM_QSTR( MP_QSTR_flush                         ), MP_ROM_PTR( &framebuffer_flush_obj                )}, //Flush the buffer to the display
	{MP_ROM_QSTR( MP_QSTR_size                          ), MP_ROM_PTR( &framebuffer_size_obj                 )}, //Get the size (width and height) of the framebuffer or a window
	{MP_ROM_QSTR( MP_QSTR_width                         ), MP_ROM_PTR( &framebuffer_width_obj                )}, //Get the width of the framebuffer or a window
	{MP_ROM_QSTR( MP_QSTR_height                        ), MP_ROM_PTR( &framebuffer_height_obj               )}, //Get the height of the framebuffer or a window
	{MP_ROM_QSTR( MP_QSTR_backlight                     ), MP_ROM_PTR( &framebuffer_backlight_obj            )}, //Get or set the backlight brightness level
	
	/* Functions: orientation */
	{MP_ROM_QSTR( MP_QSTR_orientation                   ), MP_ROM_PTR( &framebuffer_orientation_obj          )}, //Get or set the orientation
	
	/* Functions: text */
	{MP_ROM_QSTR( MP_QSTR_getTextWidth                  ), MP_ROM_PTR( &framebuffer_get_text_width_obj       )}, //Get the width a string would take
	{MP_ROM_QSTR( MP_QSTR_getTextHeight                 ), MP_ROM_PTR( &framebuffer_get_text_height_obj      )}, //Get the height a string would take
	{MP_ROM_QSTR( MP_QSTR_drawText                      ), MP_ROM_PTR( &framebuffer_draw_text_obj            )}, //Draw text
	{MP_ROM_QSTR( MP_QSTR_listFonts                     ), MP_ROM_PTR( &framebuffer_list_fonts_obj           )}, //List fonts
	{MP_ROM_QSTR( MP_QSTR_setDefaultFont                ), MP_ROM_PTR( &framebuffer_set_default_font_obj     )}, //Set default font
	
	/* Functions: PNG images */
	{MP_ROM_QSTR( MP_QSTR_pngInfo                       ), MP_ROM_PTR( &framebuffer_png_info_obj             )}, //Get information about a PNG image
	{MP_ROM_QSTR( MP_QSTR_drawPng                       ), MP_ROM_PTR( &framebuffer_draw_png_obj             )}, //Display a PNG image
	
	/* Functions: drawing */
	{MP_ROM_QSTR( MP_QSTR_getPixel                      ), MP_ROM_PTR( &framebuffer_get_pixel_obj            )}, //Get the color of a pixel
	{MP_ROM_QSTR( MP_QSTR_drawPixel                     ), MP_ROM_PTR( &framebuffer_draw_pixel_obj           )}, //Set the color of a pixel
	{MP_ROM_QSTR( MP_QSTR_drawFill                      ), MP_ROM_PTR( &framebuffer_draw_fill_obj            )}, //Fill the framebuffer or a window
	{MP_ROM_QSTR( MP_QSTR_drawLine                      ), MP_ROM_PTR( &framebuffer_draw_line_obj            )}, //Draw a line
#ifdef CONFIG_G_NEW_QUAD
	{MP_ROM_QSTR( MP_QSTR_drawQuad                      ), MP_ROM_PTR( &framebuffer_draw_quad_obj            )}, //Draw a quad
#endif
	{MP_ROM_QSTR( MP_QSTR_drawRect                      ), MP_ROM_PTR( &framebuffer_draw_rect_obj            )}, //Draw a rectangle
#ifdef CONFIG_G_NEW_TRIANGLE
	{MP_ROM_QSTR( MP_QSTR_drawTriangle                  ), MP_ROM_PTR( &framebuffer_draw_triangle_obj        )}, //Draw a triangle
	{MP_ROM_QSTR( MP_QSTR_drawTri                       ), MP_ROM_PTR( &framebuffer_draw_triangle_obj        )}, //Draw a triangle
#endif
	{MP_ROM_QSTR( MP_QSTR_drawCircle                    ), MP_ROM_PTR( &framebuffer_draw_circle_obj          )}, //Draw a circle
	{MP_ROM_QSTR( MP_QSTR_drawRaw                       ), MP_ROM_PTR( &framebuffer_draw_raw_obj             )}, //Write raw data to the buffer
	
	/* Functions: compositor windows */
	{MP_ROM_QSTR( MP_QSTR_windowCreate                  ), MP_ROM_PTR( &framebuffer_window_create_obj        )}, //Create a new window
	{MP_ROM_QSTR( MP_QSTR_windowRemove                  ), MP_ROM_PTR( &framebuffer_window_remove_obj        )}, //Delete a window
	{MP_ROM_QSTR( MP_QSTR_windowMove                    ), MP_ROM_PTR( &framebuffer_window_move_obj          )}, //Move a window
	{MP_ROM_QSTR( MP_QSTR_windowHide                    ), MP_ROM_PTR( &framebuffer_window_hide_obj          )}, //Hide a window
	{MP_ROM_QSTR( MP_QSTR_windowShow                    ), MP_ROM_PTR( &framebuffer_window_show_obj          )}, //Show a window
	{MP_ROM_QSTR( MP_QSTR_windowVisibility              ), MP_ROM_PTR( &framebuffer_window_visiblity_obj     )}, //Get or set the visibility of a window
	{MP_ROM_QSTR( MP_QSTR_windowFocus                   ), MP_ROM_PTR( &framebuffer_window_focus_obj         )}, //Bring a window to the front
	{MP_ROM_QSTR( MP_QSTR_windowResize                  ), MP_ROM_PTR( &framebuffer_window_resize_obj        )}, //Resize a window
	{MP_ROM_QSTR( MP_QSTR_windowList                    ), MP_ROM_PTR( &framebuffer_window_list_obj          )}, //List all windows

#ifdef CONFIG_G_MATRIX_ENABLE
	/* Functions: matrix stack */
	{MP_ROM_QSTR( MP_QSTR_pushMatrix                    ), MP_ROM_PTR( &framebuffer_pushMatrix_obj           )}, //Push the current matrix onto the stack
	{MP_ROM_QSTR( MP_QSTR_popMatrix                     ), MP_ROM_PTR( &framebuffer_popMatrix_obj            )}, //Pop the top matrix off the stack
	{MP_ROM_QSTR( MP_QSTR_clearMatrix                   ), MP_ROM_PTR( &framebuffer_clearMatrix_obj          )}, //Clear the matrix stack
	{MP_ROM_QSTR( MP_QSTR_matrixSize                    ), MP_ROM_PTR( &framebuffer_matrixSize_obj           )}, //Get the size of the matrix stack
	{MP_ROM_QSTR( MP_QSTR_getMatrix                     ), MP_ROM_PTR( &framebuffer_getMatrix_obj            )}, //Get the current matrix
	{MP_ROM_QSTR( MP_QSTR_setMatrix                     ), MP_ROM_PTR( &framebuffer_setMatrix_obj            )}, //Set the current matrix
	{MP_ROM_QSTR( MP_QSTR_transformPoint                ), MP_ROM_PTR( &framebuffer_transformPoint_obj       )}, //Transform a point by the current matrix
	{MP_ROM_QSTR( MP_QSTR_translate                     ), MP_ROM_PTR( &framebuffer_translate_obj            )}, //Translate (move) the canvas
	{MP_ROM_QSTR( MP_QSTR_rotate                        ), MP_ROM_PTR( &framebuffer_rotate_obj               )}, //Rotate the canvas around the origin
	{MP_ROM_QSTR( MP_QSTR_scale                         ), MP_ROM_PTR( &framebuffer_scale_obj                )}, //Scale the canvas
#endif
};

static MP_DEFINE_CONST_DICT(framebuffer_module_globals, framebuffer_module_globals_table);

const mp_obj_module_t framebuffer_module = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *)&framebuffer_module_globals,
};

#endif //CONFIG_DRIVER_FRAMEBUFFER_ENABLE
