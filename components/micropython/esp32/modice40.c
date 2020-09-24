#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "py/obj.h"

#include <driver_ice40.h>

#ifdef CONFIG_DRIVER_ICE40_ENABLE

static mp_obj_t ice40_get_done() {
    return mp_obj_new_bool(driver_ice40_get_done());
}

static mp_obj_t ice40_load_bitstream(mp_uint_t n_args, const mp_obj_t *args) {
	if (!MP_OBJ_IS_TYPE(args[0], &mp_type_bytes)) {
		mp_raise_ValueError("Expected a bytestring like object");
		return mp_const_none;
	}
	mp_uint_t length;
	uint8_t* data = (uint8_t*) mp_obj_str_get_data(args[0], &length);
    esp_err_t res = driver_ice40_load_bitstream(data, length);
    if (res != ESP_OK) mp_raise_ValueError("Failed to load bitstream");
    return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_0          ( ice40_get_done_obj,             ice40_get_done       );
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( ice40_load_bitstream_obj, 1, 1, ice40_load_bitstream );

static const mp_rom_map_elem_t ice40_module_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR_done), MP_ROM_PTR(&ice40_get_done_obj)},       //ice40.done()
    {MP_ROM_QSTR(MP_QSTR_load), MP_ROM_PTR(&ice40_load_bitstream_obj)}, //ice40.load(bitstream)
};

static MP_DEFINE_CONST_DICT(ice40_module_globals, ice40_module_globals_table);

const mp_obj_module_t ice40_module = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *) &ice40_module_globals,
};

#endif
