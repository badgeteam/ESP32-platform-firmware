#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include <buses.h>

#ifdef CONFIG_BUS_I2C0_ENABLE

STATIC mp_obj_t i2c_read_reg_(mp_obj_t _addr, mp_obj_t _reg, mp_obj_t _len) {
	int addr = mp_obj_get_int(_addr);
	int reg  = mp_obj_get_int(_reg);
	int len  = mp_obj_get_int(_len);

	if (addr < 0 || addr > 127) {
		mp_raise_msg(&mp_type_AttributeError, "I2C address out of range");
	}

	if (reg < 0 || reg > 255) {
		mp_raise_msg(&mp_type_AttributeError, "I2C register out of range");
	}

	if (len < 0) {
		mp_raise_msg(&mp_type_AttributeError, "requested negative amount of bytes");
	}

	vstr_t vstr;
	vstr_init_len(&vstr, len);

	esp_err_t res = driver_i2c_read_reg(0, addr, reg, (uint8_t *) vstr.buf, len);
	if (res != ESP_OK) {
		mp_raise_OSError(MP_EIO);
	}

	return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(i2c_read_reg_obj, i2c_read_reg_);

STATIC mp_obj_t i2c_write_reg_(mp_obj_t _addr, mp_obj_t _reg, mp_obj_t _data) {
	int addr = mp_obj_get_int(_addr);
	int reg  = mp_obj_get_int(_reg);
	mp_uint_t data_len;
	uint8_t *data = (uint8_t *) mp_obj_str_get_data(_data, &data_len);

	if (addr < 0 || addr > 127) {
		mp_raise_msg(&mp_type_AttributeError, "I2C address out of range");
	}

	if (reg < 0 || reg > 255) {
		mp_raise_msg(&mp_type_AttributeError, "I2C register out of range");
	}

	bool is_bytes = MP_OBJ_IS_TYPE(_data, &mp_type_bytes);
	if (!is_bytes) {
		mp_raise_msg(&mp_type_AttributeError, "Data should be a bytestring");
	}

	if (data_len != 1) {
		mp_raise_msg(&mp_type_AttributeError, "Data-lengths other than 1 byte are not supported");
	}

	esp_err_t res = driver_i2c_write_reg(0, addr, reg, data[0]);
	if (res != ESP_OK) {
		mp_raise_OSError(MP_EIO);
	}

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(i2c_write_reg_obj, i2c_write_reg_);

STATIC const mp_rom_map_elem_t i2c_module_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR_i2c_read_reg), MP_ROM_PTR(&i2c_read_reg_obj)},
    {MP_ROM_QSTR(MP_QSTR_i2c_write_reg), MP_ROM_PTR(&i2c_write_reg_obj)},
    {MP_ROM_QSTR(MP_QSTR_GPIO_SDA), MP_ROM_INT(CONFIG_PIN_NUM_I2C0_DATA) },
    {MP_ROM_QSTR(MP_QSTR_GPIO_CLK), MP_ROM_INT(CONFIG_PIN_NUM_I2C0_CLK) },
    {MP_ROM_QSTR(MP_QSTR_SPEED), MP_ROM_INT(CONFIG_I2C0_MASTER_FREQ_HZ) },
};

STATIC MP_DEFINE_CONST_DICT(i2c_module_globals, i2c_module_globals_table);

const mp_obj_module_t i2c_module = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&i2c_module_globals,
};

#endif
