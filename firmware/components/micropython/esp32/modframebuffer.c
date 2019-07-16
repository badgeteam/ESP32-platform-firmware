#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include <driver_framebuffer.h>

#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE

static mp_obj_t framebuffer_set_orientation(mp_uint_t n_args, const mp_obj_t *args) {
	if (n_args > 0){
		int a = mp_obj_get_int(args[0]);
		driver_framebuffer_set_orientation(a);
	}
	return mp_obj_new_int(driver_framebuffer_get_orientation());
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_set_orientation_obj, 0, 1, framebuffer_set_orientation);

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

static mp_obj_t framebuffer_clear(mp_uint_t n_args, const mp_obj_t *args)
{
	int color = n_args == 0 ? COLOR_WHITE : mp_obj_get_int(args[0]);
	driver_framebuffer_fill(color);
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_clear_obj, 0, 1, framebuffer_clear);

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

static mp_obj_t framebuffer_get_dirty()
{
	return mp_obj_new_bool(driver_framebuffer_is_dirty());	
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(framebuffer_get_dirty_obj, framebuffer_get_dirty);

static mp_obj_t framebuffer_set_greyscale(mp_obj_t value_in)
{
        driver_framebuffer_set_greyscale(mp_obj_get_int(value_in));
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(framebuffer_set_greyscale_obj, framebuffer_set_greyscale);

static const mp_rom_map_elem_t framebuffer_module_globals_table[] = {
	{MP_ROM_QSTR(MP_QSTR_orientation), MP_ROM_PTR(&framebuffer_set_orientation_obj)},
	{MP_ROM_QSTR(MP_QSTR_get), MP_ROM_PTR(&framebuffer_get_pixel_obj)},
	{MP_ROM_QSTR(MP_QSTR_set), MP_ROM_PTR(&framebuffer_set_pixel_obj)},
	{MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&framebuffer_clear_obj)},
	{MP_ROM_QSTR(MP_QSTR_flush), MP_ROM_PTR(&framebuffer_flush_obj)},
	{MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&framebuffer_char_obj)},
	{MP_ROM_QSTR(MP_QSTR_print), MP_ROM_PTR(&framebuffer_print_obj)},
	{MP_ROM_QSTR(MP_QSTR_line), MP_ROM_PTR(&framebuffer_line_obj)},
	{MP_ROM_QSTR(MP_QSTR_dirty), MP_ROM_PTR(&framebuffer_get_dirty_obj)},
	{MP_ROM_QSTR(MP_QSTR_greyscale), MP_ROM_PTR(&framebuffer_set_greyscale_obj)},
};

static MP_DEFINE_CONST_DICT(framebuffer_module_globals, framebuffer_module_globals_table);

const mp_obj_module_t framebuffer_module = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *)&framebuffer_module_globals,
};

#endif //CONFIG_DRIVER_FRAMEBUFFER_ENABLE
