#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include "extmod/vfs.h"
#include "extmod/vfs_native.h"

#include <driver_framebuffer.h>

#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE

static mp_obj_t framebuffer_orientation(mp_uint_t n_args, const mp_obj_t *args) {
	if (n_args > 0){
		int a = mp_obj_get_int(args[0]);
		driver_framebuffer_set_orientation(a);
	}
	return mp_obj_new_int(driver_framebuffer_get_orientation());
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_orientation_obj, 0, 1, framebuffer_orientation);

static mp_obj_t framebuffer_cursor(mp_uint_t n_args, const mp_obj_t *args) {
	int16_t x, y;
	if (n_args == 1) {
		//To-do: accept tuple
	} else if (n_args > 1){
                x = mp_obj_get_int(args[0]);
                y = mp_obj_get_int(args[1]);
                driver_framebuffer_setCursor(x,y);
        }
	mp_obj_t tuple[2];
        driver_framebuffer_getCursor(&x,&y);
	tuple[0] = mp_obj_new_int(x);
	tuple[1] = mp_obj_new_int(y);
	return mp_obj_new_tuple(2, tuple);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_cursor_obj, 0, 2, framebuffer_cursor);

static mp_obj_t framebuffer_get_pixel(mp_obj_t x_in, mp_obj_t y_in) {
	int x = mp_obj_get_int(x_in);
	int y = mp_obj_get_int(y_in);
	return mp_obj_new_int(driver_framebuffer_getPixel(x,y));
}
static MP_DEFINE_CONST_FUN_OBJ_2(framebuffer_get_pixel_obj, framebuffer_get_pixel);

static mp_obj_t framebuffer_set_pixel(mp_obj_t x_in, mp_obj_t y_in, mp_obj_t color_in) {
	int x = mp_obj_get_int(x_in);
	int y = mp_obj_get_int(y_in);
	int color = mp_obj_get_int(color_in);
	driver_framebuffer_setPixel(x,y,color);
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(framebuffer_set_pixel_obj, framebuffer_set_pixel);

static mp_obj_t framebuffer_set_text_color(mp_uint_t n_args, const mp_obj_t *args) {
	if (n_args == 1) {
		uint32_t color = mp_obj_get_int(args[0]);
		driver_framebuffer_setTextColor(color);
	}
	return mp_obj_new_int(driver_framebuffer_getTextColor());
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_set_text_color_obj, 0, 1, framebuffer_set_text_color);

static mp_obj_t framebuffer_fill(mp_uint_t n_args, const mp_obj_t *args)
{
	int color = n_args == 0 ? COLOR_WHITE : mp_obj_get_int(args[0]);
	driver_framebuffer_fill(color);
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_fill_obj, 0, 1, framebuffer_fill);

static mp_obj_t framebuffer_flush()
{
	driver_framebuffer_flush();
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(framebuffer_flush_obj, framebuffer_flush);

static mp_obj_t framebuffer_get_width()
{
	return mp_obj_new_int(driver_framebuffer_getWidth());
}
static MP_DEFINE_CONST_FUN_OBJ_0(framebuffer_get_width_obj, framebuffer_get_width);

static mp_obj_t framebuffer_get_height()
{
	return mp_obj_new_int(driver_framebuffer_getHeight());
}
static MP_DEFINE_CONST_FUN_OBJ_0(framebuffer_get_height_obj, framebuffer_get_height);

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
	driver_framebuffer_line(x0, y0, x1, y1, color);
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
        driver_framebuffer_rect(x, y, w, h, fill, color);
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
        driver_framebuffer_circle(x, y, r, a0, a1, fill, color);
        return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_circle_obj, 7, 7, framebuffer_circle);

static mp_obj_t framebuffer_get_dirty()
{
	if (driver_framebuffer_is_dirty()) {
		int16_t x0, y0, x1, y1;
		driver_framebuffer_get_dirty(&x0, &y0, &x1, &y1);
		mp_obj_t tuple[2];
		mp_obj_t tuple0[2];
		mp_obj_t tuple1[2];
		tuple0[0] = mp_obj_new_int(x0);
		tuple0[1] = mp_obj_new_int(y0);
		tuple[0] = mp_obj_new_tuple(2, tuple0);
		tuple1[0] = mp_obj_new_int(x1);
		tuple1[1] = mp_obj_new_int(y1);
		tuple[1] = mp_obj_new_tuple(2, tuple1);
		return mp_obj_new_tuple(2, tuple);
	}

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(framebuffer_get_dirty_obj, framebuffer_get_dirty);

static mp_obj_t framebuffer_set_greyscale(mp_obj_t value_in)
{
        driver_framebuffer_set_greyscale(mp_obj_get_int(value_in));
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(framebuffer_set_greyscale_obj, framebuffer_set_greyscale);

static mp_obj_t framebuffer_raw(mp_uint_t n_args, const mp_obj_t *args)
{
	int16_t x = mp_obj_get_int(args[0]);
	int16_t y = mp_obj_get_int(args[1]);
	int16_t w = mp_obj_get_int(args[2]);
	int16_t h = mp_obj_get_int(args[3]);
	
	mp_uint_t len;
	if (!MP_OBJ_IS_TYPE(args[4], &mp_type_bytes)) {
		mp_raise_ValueError("Expected a bytestring like object.");
		return mp_const_none;
	}
	uint8_t *data = (uint8_t *)mp_obj_str_get_data(args[4], &len);
	
	for (int16_t px = 0; px < w; px++) {
		for (int16_t py = 0; py < h; py++) {
			driver_framebuffer_setPixel(x+px,y+py,data[(x+px) + (y+py)*w]);
		}
	}
	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_raw_obj, 5, 5, framebuffer_raw);

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
		renderRes = driver_framebuffer_png(x, y, reader, mr);
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
		renderRes = driver_framebuffer_png(x, y, reader, fr);
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
		if (mr == NULL)
		{
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
		if (fr == NULL)
		{
			nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "Could not open file '%s'!",filename));
			return mp_const_none;
		}
		reader = (lib_reader_read_t) &lib_file_read;
		reader_p = fr;
	}

	struct lib_png_reader *pr = lib_png_new(reader, reader_p);
	if (pr == NULL)
	{
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

	if (res < 0)
	{
		nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_ValueError, "failed to load image: res = %d", res));
	}

	return mp_obj_new_tuple(4, tuple);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(framebuffer_png_info_obj, framebuffer_png_info);

static const mp_rom_map_elem_t framebuffer_module_globals_table[] = {
	{MP_ROM_QSTR(MP_QSTR_orientation), MP_ROM_PTR(&framebuffer_orientation_obj)},
	{MP_ROM_QSTR(MP_QSTR_cursor), MP_ROM_PTR(&framebuffer_cursor_obj)},
	{MP_ROM_QSTR(MP_QSTR_get), MP_ROM_PTR(&framebuffer_get_pixel_obj)},
	{MP_ROM_QSTR(MP_QSTR_set), MP_ROM_PTR(&framebuffer_set_pixel_obj)},
	{MP_ROM_QSTR(MP_QSTR_textColor), MP_ROM_PTR(&framebuffer_set_text_color_obj)},
	{MP_ROM_QSTR(MP_QSTR_fill), MP_ROM_PTR(&framebuffer_fill_obj)},
	{MP_ROM_QSTR(MP_QSTR_flush), MP_ROM_PTR(&framebuffer_flush_obj)},
	{MP_ROM_QSTR(MP_QSTR_print), MP_ROM_PTR(&framebuffer_print_obj)},
	{MP_ROM_QSTR(MP_QSTR_font), MP_ROM_PTR(&framebuffer_font_obj)},
	{MP_ROM_QSTR(MP_QSTR_line), MP_ROM_PTR(&framebuffer_line_obj)},
	{MP_ROM_QSTR(MP_QSTR_rect), MP_ROM_PTR(&framebuffer_rect_obj)},
	{MP_ROM_QSTR(MP_QSTR_circle), MP_ROM_PTR(&framebuffer_circle_obj)},
	{MP_ROM_QSTR(MP_QSTR_dirty), MP_ROM_PTR(&framebuffer_get_dirty_obj)},
	{MP_ROM_QSTR(MP_QSTR_greyscale), MP_ROM_PTR(&framebuffer_set_greyscale_obj)},
	{MP_ROM_QSTR(MP_QSTR_raw), MP_ROM_PTR(&framebuffer_raw_obj)},
	{MP_ROM_QSTR(MP_QSTR_png), MP_ROM_PTR(&framebuffer_png_obj)},
	{MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&framebuffer_get_width_obj)},
	{MP_ROM_QSTR(MP_QSTR_height), MP_ROM_PTR(&framebuffer_get_height_obj)},
	{MP_ROM_QSTR(MP_QSTR_png_info), MP_ROM_PTR(&framebuffer_png_info_obj)},
	{MP_ROM_QSTR(MP_QSTR_get_string_width), MP_ROM_PTR(&framebuffer_get_string_width_obj)},
	{MP_ROM_QSTR(MP_QSTR_get_string_height), MP_ROM_PTR(&framebuffer_get_string_height_obj)},
};

static MP_DEFINE_CONST_DICT(framebuffer_module_globals, framebuffer_module_globals_table);

const mp_obj_module_t framebuffer_module = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *)&framebuffer_module_globals,
};

#endif //CONFIG_DRIVER_FRAMEBUFFER_ENABLE
