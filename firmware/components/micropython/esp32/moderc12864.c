#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include <driver_erc12864.h>

#ifdef CONFIG_DRIVER_ERC12864_ENABLE

static mp_obj_t erc12864_write(mp_obj_t _data) {
	mp_uint_t len;
	if (!MP_OBJ_IS_TYPE(_data, &mp_type_bytes)) {
		mp_raise_ValueError("Expected a bytestring like object.");
		return mp_const_none;
	}
	uint8_t *data = (uint8_t *)mp_obj_str_get_data(_data, &len);
	if (len != ERC12864_BUFFER_SIZE) {
		mp_raise_ValueError("Wrong buffer size!");
		return mp_const_none;
	}
	driver_erc12864_write(data);
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(erc12864_write_obj, erc12864_write);

static mp_obj_t erc12864_flip(mp_obj_t _flip) {
	bool flip = mp_obj_get_int(_flip);
	driver_erc12864_set_rotation(flip);
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(erc12864_flip_obj, erc12864_flip);

static const mp_rom_map_elem_t erc12864_module_globals_table[] = {
	{MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&erc12864_write_obj)},
	{MP_ROM_QSTR(MP_QSTR_set_rotatation), MP_ROM_PTR(&erc12864_flip_obj)},
};

static MP_DEFINE_CONST_DICT(erc12864_module_globals, erc12864_module_globals_table);

const mp_obj_module_t erc12864_module = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *)&erc12864_module_globals,
};

#endif // CONFIG_DRIVER_ERC12864_ENABLE
