#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include "extmod/vfs.h"
#include "extmod/vfs_native.h"

#include <driver_framebuffer.h>
#include <driver_framebuffer_devices.h>

#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE

static mp_obj_t framebuffer_flush(mp_uint_t n_args, const mp_obj_t *args)
{
	uint32_t flags = 0;
	
	if (n_args > 0) {
		flags = mp_obj_get_int(args[0]);
	}
	
	driver_framebuffer_flush(flags);
	return mp_const_none;
}

static mp_obj_t framebuffer_get_size(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	
	if ((n_args > 0) && (MP_OBJ_IS_STR(args[0]))) {
		window = driver_framebuffer_find_window(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found.");
			return mp_const_none;
		}
	}
		
	int16_t width, height;
	driver_framebuffer_window_getSize(window, &width, &height);
	
	mp_obj_t tuple[2];
	tuple[0] = mp_obj_new_int(width);
	tuple[1] = mp_obj_new_int(height);
	return mp_obj_new_tuple(2, tuple);
}

static mp_obj_t framebuffer_get_width(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	
	if ((n_args > 0) && (MP_OBJ_IS_STR(args[0]))) {
		window = driver_framebuffer_find_window(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found.");
			return mp_const_none;
		}
	}
	
	return mp_obj_new_int(driver_framebuffer_getWidth(window));
}

static mp_obj_t framebuffer_get_height(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	
	if ((n_args > 0) && (MP_OBJ_IS_STR(args[0]))) {
		window = driver_framebuffer_find_window(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found.");
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
	
	if ((n_args > 1) && (!MP_OBJ_IS_INT(args[0]))) {
		//Second argument is not an integer, return error message.
		mp_raise_ValueError("Expected the second argument to be the orientation to set (integer).");
		return mp_const_none;
	}
	
	if ((n_args > 0) && (MP_OBJ_IS_STR(args[0]))) {
		//First argument is a string: we're operating on a window
		window = driver_framebuffer_find_window(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found.");
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

static mp_obj_t framebuffer_raw(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	
	if (n_args > 5) { //A window was provided
		window = driver_framebuffer_find_window(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found.");
			return mp_const_none;
		}
	}
	
	int16_t x = mp_obj_get_int(args[n_args-5]);
	int16_t y = mp_obj_get_int(args[n_args-4]);
	int16_t w = mp_obj_get_int(args[n_args-3]);
	int16_t h = mp_obj_get_int(args[n_args-2]);
	
	if (!MP_OBJ_IS_TYPE(args[n_args-1], &mp_type_bytes)) {
		mp_raise_ValueError("Expected a bytestring like object.");
		return mp_const_none;
	}
	
	mp_uint_t len;
	uint8_t *data = (uint8_t *)mp_obj_str_get_data(args[4], &len);
	
	for (int16_t px = 0; px < w; px++) {
		for (int16_t py = 0; py < h; py++) {
			driver_framebuffer_setPixel(window ? window->frames : NULL, x+px, y+py, data[(x+px) + (y+py)*w]); //FIXME: get a specific frame
		}
	}
	
	return mp_const_none;
}

static mp_obj_t framebuffer_get_pixel(mp_uint_t n_args, const mp_obj_t *args) {
	Window* window = NULL;
	
	if ((n_args > 0) && (MP_OBJ_IS_STR(args[0]))) { //A window was provided
		window = driver_framebuffer_find_window(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found.");
			return mp_const_none;
		}
	}
	
	int x = mp_obj_get_int(args[n_args-2]);
	int y = mp_obj_get_int(args[n_args-1]);
	
	return mp_obj_new_int(driver_framebuffer_getPixel(window ? window->frames : NULL, x, y)); //FIXME: get a specific frame
}

static mp_obj_t framebuffer_set_pixel(mp_uint_t n_args, const mp_obj_t *args) {
	Window* window = NULL;
	
	if ((n_args > 0) && (MP_OBJ_IS_STR(args[0]))) { //A window was provided
		window = driver_framebuffer_find_window(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found.");
			return mp_const_none;
		}
	}
	
	int x = mp_obj_get_int(args[n_args-3]);
	int y = mp_obj_get_int(args[n_args-2]);
	int color = mp_obj_get_int(args[n_args-1]);
	
	driver_framebuffer_setPixel(window ? window->frames : NULL, x, y, color); //FIXME: get a specific frame
	return mp_const_none;
}

static mp_obj_t framebuffer_fill(mp_uint_t n_args, const mp_obj_t *args)
{
	Window* window = NULL;
	int color = COLOR_FILL_DEFAULT;
	
	if ((n_args > 0) && (!(MP_OBJ_IS_STR(args[0]) || MP_OBJ_IS_INT(args[0])))) {
		//First argument is not a string or integer, return error message.
		mp_raise_ValueError("Expected the first argument to be either the name of a window (string) or the color to fill with (integer).");
		return mp_const_none;
	}
	
	if ((n_args > 1) && (!MP_OBJ_IS_INT(args[0]))) {
		//Second argument is not an integer, return error message.
		mp_raise_ValueError("Expected the second argument to be the color to fill with (integer).");
		return mp_const_none;
	}
	
	if ((n_args > 0) && (MP_OBJ_IS_STR(args[0]))) {
		window = driver_framebuffer_find_window(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found.");
			return mp_const_none;
		}
	}
	
	if ((n_args > 0) && (MP_OBJ_IS_INT(args[n_args-1]))) {
		//Last argument is the color as an integer
		color = mp_obj_get_int(args[n_args-1]);
	}
	
	driver_framebuffer_fill(window ? window->frames : NULL, color); //FIXME: get a specific frame
	return mp_const_none;
}

static mp_obj_t framebuffer_cursor(mp_uint_t n_args, const mp_obj_t *args) {
	Window* window = NULL;
	int16_t x, y;
	
	if ((n_args > 0) && (!(MP_OBJ_IS_STR(args[0]) || MP_OBJ_IS_INT(args[0]) || MP_OBJ_IS_TYPE(args[0], &mp_type_tuple)))) {
		//First argument is not a string or integer, return error message.
		mp_raise_ValueError("Expected the first argument to be either the name of a window (string), the x coordinate (integer) or a tuple containing the x and y coordinate (x,y).");
		return mp_const_none;
	}
	
	if ((n_args == 1) && MP_OBJ_IS_INT(args[0])) {
		mp_raise_ValueError("Expected two coordinates: x and y.");
		return mp_const_none;
	}
	
	if ((n_args > 2) && (MP_OBJ_IS_TYPE(args[n_args-2], &mp_type_tuple) || MP_OBJ_IS_TYPE(args[n_args-1], &mp_type_tuple))) {
		mp_raise_ValueError("Expected the last argument(s) to be either a tuple (x,y) or two integers x and y.");
		return mp_const_none;
	}
	
	if ((n_args > 0) && (MP_OBJ_IS_STR(args[0]))) {
		window = driver_framebuffer_find_window(mp_obj_str_get_str(args[0]));
		if (!window) {
			mp_raise_ValueError("Window not found.");
			return mp_const_none;
		}
	}
		
	if (((n_args == 1) || (n_args == 2)) && MP_OBJ_IS_TYPE(args[n_args-1], &mp_type_tuple)) {
        mp_obj_t *items; uint len;
        mp_obj_tuple_get(args[n_args-1], &len, &items);
		if (len != 2) {
			mp_raise_ValueError("Expected the tuple to contain two items (x,y).");
			return mp_const_none;
		}
		x = mp_obj_get_int(items[0]);
		y = mp_obj_get_int(items[1]);
		driver_framebuffer_setCursor(x,y); //FIXME
		return mp_const_none;
	} else if ((n_args > 1) && MP_OBJ_IS_INT(args[n_args-2]) && MP_OBJ_IS_INT(args[n_args-1])) {
		x = mp_obj_get_int(args[n_args-2]);
		y = mp_obj_get_int(args[n_args-2]);
		driver_framebuffer_setCursor(x,y); //FIXME
		return mp_const_none;
	}
	
	mp_obj_t tuple[2];
	driver_framebuffer_getCursor(&x,&y); //FIXME
	tuple[0] = mp_obj_new_int(x);
	tuple[1] = mp_obj_new_int(y);
	return mp_obj_new_tuple(2, tuple);
}

static mp_obj_t framebuffer_set_text_color(mp_uint_t n_args, const mp_obj_t *args) {
	if (n_args == 1) {
		uint32_t color = mp_obj_get_int(args[0]);
		driver_framebuffer_setTextColor(color);
	}
	return mp_obj_new_int(driver_framebuffer_getTextColor());
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_set_text_color_obj, 0, 1, framebuffer_set_text_color);

STATIC mp_obj_t framebuffer_print(mp_obj_t text_in) {
	const char *text = mp_obj_str_get_str(text_in);
	driver_framebuffer_print(text);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(framebuffer_print_obj, framebuffer_print);

STATIC mp_obj_t framebuffer_get_string_width(mp_obj_t text_in) {
	const char *text = mp_obj_str_get_str(text_in);
	int value = driver_framebuffer_get_string_width(text);
	return mp_obj_new_int(value);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(framebuffer_get_string_width_obj, framebuffer_get_string_width);

STATIC mp_obj_t framebuffer_get_string_height(mp_obj_t text_in) {
	const char *text = mp_obj_str_get_str(text_in);
	int value = driver_framebuffer_get_string_height(text);
	return mp_obj_new_int(value);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(framebuffer_get_string_height_obj, framebuffer_get_string_height);

STATIC mp_obj_t framebuffer_font(mp_obj_t name_in) {
	const char *name = mp_obj_str_get_str(name_in);
	return mp_obj_new_bool(driver_framebuffer_selectFont(name));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(framebuffer_font_obj, framebuffer_font);

static mp_obj_t framebuffer_line(mp_uint_t n_args, const mp_obj_t *args)
{
	int x0 =  mp_obj_get_int(args[0]);
	int y0 =  mp_obj_get_int(args[1]);
	int x1 =  mp_obj_get_int(args[2]);
	int y1 =  mp_obj_get_int(args[3]);
	int color =  mp_obj_get_int(args[4]);
	driver_framebuffer_line(NULL, x0, y0, x1, y1, color);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_line_obj, 5, 5, framebuffer_line);

static mp_obj_t framebuffer_rect(mp_uint_t n_args, const mp_obj_t *args)
{
        int x     =  mp_obj_get_int(args[0]);
        int y     =  mp_obj_get_int(args[1]);
        int w     =  mp_obj_get_int(args[2]);
        int h     =  mp_obj_get_int(args[3]);
        int fill  =  mp_obj_get_int(args[4]);
        int color =  mp_obj_get_int(args[5]);
        driver_framebuffer_rect(NULL, x, y, w, h, fill, color);
        return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_rect_obj, 6, 6, framebuffer_rect);

static mp_obj_t framebuffer_circle(mp_uint_t n_args, const mp_obj_t *args)
{
        int x     =  mp_obj_get_int(args[0]);
        int y     =  mp_obj_get_int(args[1]);
        int r     =  mp_obj_get_int(args[2]);
        int a0    =  mp_obj_get_int(args[3]);
        int a1    =  mp_obj_get_int(args[4]);
        int fill  =  mp_obj_get_int(args[5]);
        int color =  mp_obj_get_int(args[6]);
        driver_framebuffer_circle(NULL, x, y, r, a0, a1, fill, color);
        return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_circle_obj, 7, 7, framebuffer_circle);

static mp_obj_t framebuffer_png(mp_uint_t n_args, const mp_obj_t *args)
{
	int16_t x = mp_obj_get_int(args[0]);
	int16_t y = mp_obj_get_int(args[1]);
	
	lib_reader_read_t reader;
	
	bool is_bytes = MP_OBJ_IS_TYPE(args[2], &mp_type_bytes);
	
	esp_err_t renderRes = ESP_FAIL;
	
	if (is_bytes) {
		mp_uint_t len;
		uint8_t *data = (uint8_t *)mp_obj_str_get_data(args[2], &len);
		struct lib_mem_reader *mr = lib_mem_new(data, len);
		if (mr == NULL) {
			mp_raise_ValueError("Out of memory");
			return mp_const_none;
		}
		reader = (lib_reader_read_t) &lib_mem_read;
		renderRes = driver_framebuffer_png(NULL, x, y, reader, mr);
		lib_mem_destroy(mr);
	} else {
		const char* filename = mp_obj_str_get_str(args[2]);
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
		renderRes = driver_framebuffer_png(NULL, x, y, reader, fr);
		lib_file_destroy(fr);
	}
	
	if (renderRes != ESP_OK) {
		mp_raise_ValueError("Rendering error");
	}
	
	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_png_obj, 3, 3, framebuffer_png);

STATIC mp_obj_t framebuffer_png_info(mp_obj_t obj_filename)

{
	lib_reader_read_t reader;
	void * reader_p;
	bool is_bytes = MP_OBJ_IS_TYPE(obj_filename, &mp_type_bytes);
	if (is_bytes) {
		size_t len;
		const uint8_t* png_data = (const uint8_t *) mp_obj_str_get_data(obj_filename, &len);
		struct lib_mem_reader *mr = lib_mem_new(png_data, len);
		if (mr == NULL) {
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "out of memory!"));
			return mp_const_none;
		}
		reader = (lib_reader_read_t) &lib_mem_read;
		reader_p = mr;
	} else {
		const char* filename = mp_obj_str_get_str(obj_filename);
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
STATIC MP_DEFINE_CONST_FUN_OBJ_1(framebuffer_png_info_obj, framebuffer_png_info);


static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_flush_obj,      0, 1, framebuffer_flush      );
/* Flush the framebuffer to the display. Arguments: flags (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_get_size_obj,   0, 1, framebuffer_get_size   );
/* Get the size (width, height) of the framebuffer or a window. Arguments: window (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_get_width_obj,  0, 1, framebuffer_get_width  );
/* Get the width of the framebuffer or a window. Arguments: window (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( framebuffer_get_height_obj, 0, 1, framebuffer_get_height );
/* Get the height of the framebuffer or a window. Arguments: window (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_orientation_obj, 0, 2, framebuffer_orientation);
/* Get or set the orientation of the framebuffer or a window. Arguments: window (optional), orientation (optional) */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_raw_obj,         5, 6, framebuffer_raw);
/* Copy a raw bytes buffer directly to the framebuffer or a window. Arguments: window (optional), x, y, width, height, data */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_get_pixel_obj, 2, 3, framebuffer_get_pixel);
/* Get the color of a pixel in the framebuffer or in a window. Arguments: window (optional), x, y */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_set_pixel_obj, 3, 4, framebuffer_set_pixel);
/* Set the color of a pixel in the framebuffer or in a window. Arguments: window (optional), x, y, color */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_fill_obj, 0, 2, framebuffer_fill);
/* Fill the framebuffer or a window with a color. Arguments: window (optional), color */

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_cursor_obj,    0, 3, framebuffer_cursor);
/* Get or set the text cursor position of the framebuffer or a window. Arguments: window (optional), location as tuple (x,y) OR location as two integers x, y (optional) */


static const mp_rom_map_elem_t framebuffer_module_globals_table[] = {
	/* Constants */
	{MP_ROM_QSTR( MP_QSTR_FLAG_FORCE                    ), MP_ROM_INT( FB_FLAG_FORCE                     )}, //Refresh even when not dirty
	{MP_ROM_QSTR( MP_QSTR_FLAG_FULL                     ), MP_ROM_INT( FB_FLAG_FULL                      )}, //Full refresh (instead of partial refresh)
	{MP_ROM_QSTR( MP_QSTR_FLAG_LUT_GREYSCALE            ), MP_ROM_INT( FB_FLAG_LUT_GREYSCALE             )}, //E-ink display: use greyscale LUT
	{MP_ROM_QSTR( MP_QSTR_FLAG_LUT_NORMAL               ), MP_ROM_INT( FB_FLAG_LUT_NORMAL                )}, //E-ink display: use normal LUT
	{MP_ROM_QSTR( MP_QSTR_FLAG_LUT_FAST                 ), MP_ROM_INT( FB_FLAG_LUT_FAST                  )}, //E-ink display: use fast LUT
	{MP_ROM_QSTR( MP_QSTR_FLAG_LUT_FASTEST              ), MP_ROM_INT( FB_FLAG_LUT_FASTEST               )}, //E-ink display: use fastest LUT
	{MP_ROM_QSTR( MP_QSTR_ORIENTATION_LANDSCAPE         ), MP_ROM_INT( 0                                 )}, //Orientation: landscape
	{MP_ROM_QSTR( MP_QSTR_ORIENTATION_PORTRAIT          ), MP_ROM_INT( 90                                )}, //Orientation: portrait
	{MP_ROM_QSTR( MP_QSTR_ORIENTATION_REVERSE_LANDSCAPE ), MP_ROM_INT( 180                               )}, //Orientation: reverse landscape
	{MP_ROM_QSTR( MP_QSTR_ORIENTATION_REVERSE_PORTRAIT  ), MP_ROM_INT( 270                               )}, //Orientation: reverse portrait
	
	/* Functions: general */
	{MP_ROM_QSTR( MP_QSTR_flush                         ), MP_ROM_PTR( &framebuffer_flush_obj            )}, //Flush the buffer to the display
	{MP_ROM_QSTR( MP_QSTR_size                          ), MP_ROM_PTR( &framebuffer_get_size_obj         )}, //Get the size (width and height) of the framebuffer or a window
	{MP_ROM_QSTR( MP_QSTR_width                         ), MP_ROM_PTR( &framebuffer_get_width_obj        )}, //Get the width of the framebuffer or a window
	{MP_ROM_QSTR( MP_QSTR_height                        ), MP_ROM_PTR( &framebuffer_get_height_obj       )}, //Get the height of the framebuffer or a window
	{MP_ROM_QSTR( MP_QSTR_orientation                   ), MP_ROM_PTR( &framebuffer_orientation_obj      )}, //Get or set the orientation
	{MP_ROM_QSTR( MP_QSTR_raw                           ), MP_ROM_PTR( &framebuffer_raw_obj              )}, //Write raw data to the buffer
	
	/* Functions: compositor */
	//{MP_ROM_QSTR( MP_QSTR_create_window                 ), MP_ROM_PTR( &framebuffer_create_window_obj    )}, //Create a new window
	//{MP_ROM_QSTR( MP_QSTR_delete_window                 ), MP_ROM_PTR( &framebuffer_delete_window_obj    )}, //Delete a window
	//{MP_ROM_QSTR( MP_QSTR_move_window                   ), MP_ROM_PTR( &framebuffer_move_window_obj      )}, //Move a window
	//{MP_ROM_QSTR( MP_QSTR_hide_window                   ), MP_ROM_PTR( &framebuffer_hide_window_obj      )}, //Hide a window
	//{MP_ROM_QSTR( MP_QSTR_show_window                   ), MP_ROM_PTR( &framebuffer_show_window_obj      )}, //Show a window
	//{MP_ROM_QSTR( MP_QSTR_window_visible                ), MP_ROM_PTR( &framebuffer_window_visible_obj   )}, //Get or set the visibility of a window
	
	/* Functions: drawing */
	{MP_ROM_QSTR( MP_QSTR_get                           ), MP_ROM_PTR( &framebuffer_get_pixel_obj        )},
	{MP_ROM_QSTR( MP_QSTR_set                           ), MP_ROM_PTR( &framebuffer_set_pixel_obj        )},
	{MP_ROM_QSTR( MP_QSTR_fill                          ), MP_ROM_PTR( &framebuffer_fill_obj             )},
	{MP_ROM_QSTR( MP_QSTR_line                          ), MP_ROM_PTR( &framebuffer_line_obj             )},
	{MP_ROM_QSTR( MP_QSTR_rect                          ), MP_ROM_PTR( &framebuffer_rect_obj             )},
	{MP_ROM_QSTR( MP_QSTR_circle                        ), MP_ROM_PTR( &framebuffer_circle_obj           )},
	
	/* Functions: text */
	{MP_ROM_QSTR( MP_QSTR_cursor                        ), MP_ROM_PTR( &framebuffer_cursor_obj           )},
	{MP_ROM_QSTR( MP_QSTR_textColor                     ), MP_ROM_PTR( &framebuffer_set_text_color_obj   )},
	{MP_ROM_QSTR( MP_QSTR_print                         ), MP_ROM_PTR( &framebuffer_print_obj            )},
	{MP_ROM_QSTR( MP_QSTR_font                          ), MP_ROM_PTR( &framebuffer_font_obj             )},
	{MP_ROM_QSTR( MP_QSTR_get_string_width              ), MP_ROM_PTR( &framebuffer_get_string_width_obj )},
	{MP_ROM_QSTR( MP_QSTR_get_string_height             ), MP_ROM_PTR( &framebuffer_get_string_height_obj)},
	
	/* Functions: format parsers */
	{MP_ROM_QSTR( MP_QSTR_png_info                      ), MP_ROM_PTR( &framebuffer_png_info_obj         )},
	{MP_ROM_QSTR( MP_QSTR_png                           ), MP_ROM_PTR( &framebuffer_png_obj              )},
};

static MP_DEFINE_CONST_DICT(framebuffer_module_globals, framebuffer_module_globals_table);

const mp_obj_module_t framebuffer_module = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *)&framebuffer_module_globals,
};

#endif //CONFIG_DRIVER_FRAMEBUFFER_ENABLE
