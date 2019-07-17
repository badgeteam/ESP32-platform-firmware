#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

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
	driver_framebuffer_pixel(x,y,color);
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(framebuffer_set_pixel_obj, framebuffer_set_pixel);

static mp_obj_t framebuffer_set_text_color(mp_obj_t color_in) {
	int color = mp_obj_get_int(color_in);
	driver_framebuffer_setTextColor(color);
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(framebuffer_set_text_color_obj, framebuffer_set_text_color);

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

static mp_obj_t framebuffer_char(mp_uint_t n_args, const mp_obj_t *args)
{
	int x0 = mp_obj_get_int(args[0]);
	int y0 = mp_obj_get_int(args[1]);
	const uint16_t data = mp_obj_get_int(args[2]);
	int scaleX = mp_obj_get_int(args[3]);
	int scaleY = mp_obj_get_int(args[4]);
	int color = mp_obj_get_int(args[5]);
	driver_framebuffer_char(x0, y0, data, scaleX, scaleY, color);
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_char_obj, 6, 6, framebuffer_char);

STATIC mp_obj_t framebuffer_print(mp_obj_t text_in) {
	const char *text = mp_obj_str_get_str(text_in);
	driver_framebuffer_print(text);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(framebuffer_print_obj, framebuffer_print);

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

uint16_t rgbTo565(uint32_t in)
{
	uint8_t r = (in>>16)&0xFF;
	uint8_t g = (in>>8)&0xFF;
	uint8_t b = in&0xFF;
	uint16_t out = ((b & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (r >> 3);
	return out;
}

static mp_obj_t framebuffer_rgbTo565(mp_obj_t value_in)
{
	int in = mp_obj_get_int(value_in);
	int out = rgbTo565(in);
	return mp_obj_new_int(out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(framebuffer_rgbTo565_obj, framebuffer_rgbTo565);

static mp_obj_t framebuffer_raw565(mp_uint_t n_args, const mp_obj_t *args)
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
	
	/*for (int16_t px = 0; px < w, px++) {
		for (int16_t py = 0; py < w, py++) {
			uint32_t pos = (x+px)*3 + (y+py)*3*w;
			driver_framebuffer_pixel(x,y,framebuffer_rgbTo565(data_in[pos],data_in[pos+1],data_in[pos+2]));
		}
	}*/
	
	for (int16_t px = 0; px < w; px++) {
		for (int16_t py = 0; py < h; py++) {
			uint32_t pos = (x+px)*2 + (y+py)*2*w;
			driver_framebuffer_pixel(x+px,y+py,(data[pos]<<8)+data[pos+1]);
		}
	}
	return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_raw565_obj, 5, 5, framebuffer_raw565);

static const mp_rom_map_elem_t framebuffer_module_globals_table[] = {
	{MP_ROM_QSTR(MP_QSTR_orientation), MP_ROM_PTR(&framebuffer_orientation_obj)},
	{MP_ROM_QSTR(MP_QSTR_cursor), MP_ROM_PTR(&framebuffer_cursor_obj)},
	{MP_ROM_QSTR(MP_QSTR_get), MP_ROM_PTR(&framebuffer_get_pixel_obj)},
	{MP_ROM_QSTR(MP_QSTR_set), MP_ROM_PTR(&framebuffer_set_pixel_obj)},
	{MP_ROM_QSTR(MP_QSTR_textColor), MP_ROM_PTR(&framebuffer_set_text_color_obj)},
	{MP_ROM_QSTR(MP_QSTR_fill), MP_ROM_PTR(&framebuffer_fill_obj)},
	{MP_ROM_QSTR(MP_QSTR_flush), MP_ROM_PTR(&framebuffer_flush_obj)},
	{MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&framebuffer_char_obj)},
	{MP_ROM_QSTR(MP_QSTR_print), MP_ROM_PTR(&framebuffer_print_obj)},
	{MP_ROM_QSTR(MP_QSTR_line), MP_ROM_PTR(&framebuffer_line_obj)},
	{MP_ROM_QSTR(MP_QSTR_rect), MP_ROM_PTR(&framebuffer_rect_obj)},
	{MP_ROM_QSTR(MP_QSTR_circle), MP_ROM_PTR(&framebuffer_circle_obj)},
	{MP_ROM_QSTR(MP_QSTR_dirty), MP_ROM_PTR(&framebuffer_get_dirty_obj)},
	{MP_ROM_QSTR(MP_QSTR_greyscale), MP_ROM_PTR(&framebuffer_set_greyscale_obj)},
	{MP_ROM_QSTR(MP_QSTR_rgbTo565), MP_ROM_PTR(&framebuffer_rgbTo565_obj)},
	{MP_ROM_QSTR(MP_QSTR_raw565), MP_ROM_PTR(&framebuffer_raw565_obj)},
};

static MP_DEFINE_CONST_DICT(framebuffer_module_globals, framebuffer_module_globals_table);

const mp_obj_module_t framebuffer_module = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *)&framebuffer_module_globals,
};

#endif //CONFIG_DRIVER_FRAMEBUFFER_ENABLE
