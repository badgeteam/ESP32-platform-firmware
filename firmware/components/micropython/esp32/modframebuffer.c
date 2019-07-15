#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include <driver_framebuffer.h>

#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE

bool     screen_flipped = false;
uint16_t screen_orientation = 0;

static uint16_t get_orientation(int a){
	if (a == 90)
		return 90;
	else if (a == 180)
		return 180;
	else if (a == 270)
		return 270;
	else
		return 0;
}

static mp_obj_t framebuffer_set_orientation(mp_uint_t n_args, const mp_obj_t *args) {
	if (n_args > 0){
		int a = mp_obj_get_int(args[0]);
		a %= 360;
		screen_flipped = false;
		if (a >= 180) {
			screen_flipped = true;
			a -= 180;
		}
		screen_orientation = get_orientation(a);
		printf("STUB set orientation to %u (flip: %s)\n", screen_orientation, screen_flipped ? "yes" : "no");
	}

	int a = screen_orientation;
	if (screen_flipped)
		a += 180;
	a %= 360;
	return mp_obj_new_int(a);
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_set_orientation_obj, 0, 1, framebuffer_set_orientation);

static mp_obj_t framebuffer_get_pixel(mp_obj_t x_in, mp_obj_t y_in) {
	int x = mp_obj_get_int(x_in);
	int y = mp_obj_get_int(y_in);
	return mp_obj_new_int(0);//gdispGetPixelColor(x,y)); //FIXME
}
static MP_DEFINE_CONST_FUN_OBJ_2(framebuffer_get_pixel_obj, framebuffer_get_pixel);

static mp_obj_t framebuffer_set_pixel(mp_obj_t x_in, mp_obj_t y_in, mp_obj_t color_in) {
	int x = mp_obj_get_int(x_in);
	int y = mp_obj_get_int(y_in);
	int color = mp_obj_get_int(color_in);
	driver_framebuffer_pixel(x,y,color);
	#ifdef FB_TYPE_24BPP
		driver_framebuffer_pixel(x,y,(color>>16)&0xFF,(color>>8)&0xFF,color&0xFF);
	#else
		driver_framebuffer_pixel(x,y,color);
	#endif
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(framebuffer_set_pixel_obj, framebuffer_set_pixel);

static mp_obj_t framebuffer_clear(mp_uint_t n_args, const mp_obj_t *args)
{
	int color = n_args == 0 ? COLOR_WHITE : mp_obj_get_int(args[0]);
	#ifdef FB_TYPE_24BPP
	driver_framebuffer_fill((color>>16)&0xFF,(color>>8)&0xFF,color&0xFF);
	#else
	driver_framebuffer_fill(color);
	#endif
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

	#ifdef FB_TYPE_24BPP
		driver_framebuffer_char(x0, y0, data, scaleX, scaleY, (color>>16)&0xFF,(color>>8)&0xFF,color&0xFF);
	#else
		driver_framebuffer_char(x0, y0, data, scaleX, scaleY, color);
	#endif

	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(framebuffer_char_obj, 6, 6, framebuffer_char);

STATIC mp_obj_t framebuffer_print (mp_obj_t text_in) {
	const char *text = mp_obj_str_get_str(text_in);
	driver_framebuffer_print(text);
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(framebuffer_print_obj, framebuffer_print);


static const mp_rom_map_elem_t framebuffer_module_globals_table[] = {
	{MP_ROM_QSTR(MP_QSTR_orientation), MP_ROM_PTR(&framebuffer_set_orientation_obj)},
	{MP_ROM_QSTR(MP_QSTR_get), MP_ROM_PTR(&framebuffer_get_pixel_obj)},
	{MP_ROM_QSTR(MP_QSTR_set), MP_ROM_PTR(&framebuffer_set_pixel_obj)},
	{MP_ROM_QSTR(MP_QSTR_clear), MP_ROM_PTR(&framebuffer_clear_obj)},
	{MP_ROM_QSTR(MP_QSTR_flush), MP_ROM_PTR(&framebuffer_flush_obj)},
	{MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&framebuffer_char_obj)},
	{MP_ROM_QSTR(MP_QSTR_print), MP_ROM_PTR(&framebuffer_print_obj)},
};

static MP_DEFINE_CONST_DICT(framebuffer_module_globals, framebuffer_module_globals_table);

const mp_obj_module_t framebuffer_module = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *)&framebuffer_module_globals,
};

#endif //CONFIG_DRIVER_FRAMEBUFFER_ENABLE
