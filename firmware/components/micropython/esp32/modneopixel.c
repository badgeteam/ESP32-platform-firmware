#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include <driver_neopixel.h>

#ifdef CONFIG_DRIVER_NEOPIXEL_ENABLE

static mp_obj_t neopixels_enable() {
	return mp_obj_new_int(driver_neopixel_enable());
}
static MP_DEFINE_CONST_FUN_OBJ_0(neopixels_enable_obj, neopixels_enable);

static mp_obj_t neopixels_disable() {
	return mp_obj_new_int(driver_neopixel_disable());
}
static MP_DEFINE_CONST_FUN_OBJ_0(neopixels_disable_obj, neopixels_disable);

static mp_obj_t neopixels_send(mp_uint_t n_args, const mp_obj_t *args) {
	bool is_bytes = MP_OBJ_IS_TYPE(args[0], &mp_type_bytes);
	if (!is_bytes) {
		mp_raise_ValueError("Expected a bytestring like object.");
		return mp_const_none;
	}
	mp_uint_t len;
	uint8_t *leds = (uint8_t *)mp_obj_str_get_data(args[0], &len);
	if (n_args > 1) {
		mp_uint_t arglen = mp_obj_get_int(args[1]);
		if (len != arglen) {
			printf("Neopixel send length mismatch (%d != %d)\n", len, arglen);
			mp_raise_ValueError("length mismatch");
			return mp_const_none;
		}
	}
	return mp_obj_new_int(driver_neopixel_send_data(leds, len));
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(neopixels_send_obj, 1,2 ,neopixels_send);

static const mp_rom_map_elem_t neopixel_module_globals_table[] = {
	{MP_OBJ_NEW_QSTR(MP_QSTR_enable), (mp_obj_t)&neopixels_enable_obj},
	{MP_OBJ_NEW_QSTR(MP_QSTR_disable), (mp_obj_t)&neopixels_disable_obj},
	{MP_OBJ_NEW_QSTR(MP_QSTR_send), (mp_obj_t)&neopixels_send_obj},
};

static MP_DEFINE_CONST_DICT(neopixel_module_globals, neopixel_module_globals_table);

const mp_obj_module_t neopixel_module = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *)&neopixel_module_globals,
};

#endif // CONFIG_DRIVER_NEOPIXEL_ENABLE
