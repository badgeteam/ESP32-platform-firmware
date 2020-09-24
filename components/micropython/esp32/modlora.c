#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include "driver_lora.h"

#ifdef CONFIG_DRIVER_LORA_ENABLE

static mp_obj_t modlora_set_header_explicit()
{
	esp_err_t res = driver_lora_explicit_header_mode();
	if (res != ESP_OK) {
		mp_raise_ValueError("Failed to set header to explcit!");
		return mp_const_none;
	}
	return mp_const_none;
}

static mp_obj_t modlora_set_header_implicit(mp_obj_t _size)
{
	uint8_t size = mp_obj_get_int(_size);
	esp_err_t res = driver_lora_implicit_header_mode(size);
	if (res != ESP_OK) {
		mp_raise_ValueError("Failed to set header to implcit!");
		return mp_const_none;
	}
	return mp_const_none;
}

static mp_obj_t modlora_set_mode_idle()
{
	esp_err_t res = driver_lora_idle();
	if (res != ESP_OK) {
		mp_raise_ValueError("Failed to set mode to idle!");
		return mp_const_none;
	}
	return mp_const_none;
}

static mp_obj_t modlora_set_mode_sleep()
{
	esp_err_t res = driver_lora_sleep();
	if (res != ESP_OK) {
		mp_raise_ValueError("Failed to set mode to sleep!");
		return mp_const_none;
	}
	return mp_const_none;
}

static mp_obj_t modlora_set_mode_receive()
{
	esp_err_t res = driver_lora_receive();
	if (res != ESP_OK) {
		mp_raise_ValueError("Failed to set mode to receive!");
		return mp_const_none;
	}
	return mp_const_none;
}

static mp_obj_t modlora_set_tx_power(mp_obj_t _tx_power)
{
	uint8_t tx_power = mp_obj_get_int(_tx_power);
	esp_err_t res = driver_lora_set_tx_power(tx_power);
	if (res != ESP_OK) {
		mp_raise_ValueError("Failed to set tx power!");
		return mp_const_none;
	}
	return mp_const_none;
}

static mp_obj_t modlora_set_frequency(mp_obj_t _frequency)
{	
	long frequency = mp_obj_get_int64(_frequency);
	esp_err_t res = driver_lora_set_frequency(frequency);
	if (res != ESP_OK) {
		mp_raise_ValueError("Failed to set frequency!");
		return mp_const_none;
	}
	return mp_const_none;
}

static mp_obj_t modlora_set_spreading_factor(mp_obj_t _spf)
{
	uint8_t spf = mp_obj_get_int(_spf);
	esp_err_t res = driver_lora_set_spreading_factor(spf);
	if (res != ESP_OK) {
		mp_raise_ValueError("Failed to set spreading factor!");
		return mp_const_none;
	}
	return mp_const_none;
}

static mp_obj_t modlora_set_bandwidth(mp_obj_t _sbw)
{	
	long sbw = mp_obj_get_int64(_sbw);
	esp_err_t res = driver_lora_set_bandwidth(sbw);
	if (res != ESP_OK) {
		mp_raise_ValueError("Failed to set bandwidth!");
		return mp_const_none;
	}
	return mp_const_none;
}

static mp_obj_t modlora_set_coding_rate(mp_obj_t _denominator)
{	
	uint8_t denominator = mp_obj_get_int(_denominator);
	esp_err_t res = driver_lora_set_coding_rate(denominator);
	if (res != ESP_OK) {
		mp_raise_ValueError("Failed to set coding rate!");
		return mp_const_none;
	}
	return mp_const_none;
}

static mp_obj_t modlora_set_preamble_length(mp_obj_t _length)
{	
	long length = mp_obj_get_int64(_length);
	esp_err_t res = driver_lora_set_preamble_length(length);
	if (res != ESP_OK) {
		mp_raise_ValueError("Failed to set preamble length!");
		return mp_const_none;
	}
	return mp_const_none;
}

static mp_obj_t modlora_set_sync_word(mp_obj_t _sw)
{	
	uint8_t sw = mp_obj_get_int(_sw);
	esp_err_t res = driver_lora_set_sync_word(sw);
	if (res != ESP_OK) {
		mp_raise_ValueError("Failed to set sync word!");
		return mp_const_none;
	}
	return mp_const_none;
}

static mp_obj_t modlora_enable_crc()
{	
	esp_err_t res = driver_lora_enable_crc();
	if (res != ESP_OK) {
		mp_raise_ValueError("Failed to enable crc!");
		return mp_const_none;
	}
	return mp_const_none;
}

static mp_obj_t modlora_disable_crc()
{	
	esp_err_t res = driver_lora_disable_crc();
	if (res != ESP_OK) {
		mp_raise_ValueError("Failed to disable crc!");
		return mp_const_none;
	}
	return mp_const_none;
}

static mp_obj_t modlora_send_packet(mp_obj_t _data)
{
	mp_uint_t len;
	if (!MP_OBJ_IS_TYPE(_data, &mp_type_bytes)) {
		mp_raise_ValueError("Expected a bytestring like object.");
		return mp_const_none;
	}
	uint8_t *data = (uint8_t *)mp_obj_str_get_data(_data, &len);
	
	esp_err_t res;
	
	res = driver_lora_send_packet(data, len);
	if (res != ESP_OK) {
		mp_raise_ValueError("Failed to transmit packet!");
		return mp_const_none;
	}
	
	printf("Done.\n");
	return mp_const_none;
}

static mp_obj_t modlora_received()
{	
	bool status;
	esp_err_t res = driver_lora_received(&status);
	if (res != ESP_OK) {
		mp_raise_ValueError("Failed to query received!");
		return mp_const_none;
	}
	return mp_obj_new_bool(status);
}

static mp_obj_t modlora_receive_packet()
{	
	uint8_t buffer[256];
	uint8_t length;
	esp_err_t res = driver_lora_receive_packet(buffer, 255, &length);
	if (res != ESP_OK) {
		mp_raise_ValueError("Failed to receive packet!");
		return mp_const_none;
	}
	return mp_obj_new_bytes(buffer, length);
}

/* --- */
static MP_DEFINE_CONST_FUN_OBJ_0(modlora_set_header_explicit_obj,  modlora_set_header_explicit);
static MP_DEFINE_CONST_FUN_OBJ_1(modlora_set_header_implicit_obj,  modlora_set_header_implicit);
static MP_DEFINE_CONST_FUN_OBJ_0(modlora_set_mode_idle_obj,        modlora_set_mode_idle);
static MP_DEFINE_CONST_FUN_OBJ_0(modlora_set_mode_sleep_obj,       modlora_set_mode_sleep);
static MP_DEFINE_CONST_FUN_OBJ_0(modlora_set_mode_receive_obj,     modlora_set_mode_receive);
static MP_DEFINE_CONST_FUN_OBJ_1(modlora_set_tx_power_obj,         modlora_set_tx_power);
static MP_DEFINE_CONST_FUN_OBJ_1(modlora_set_frequency_obj,        modlora_set_frequency);
static MP_DEFINE_CONST_FUN_OBJ_1(modlora_set_spreading_factor_obj, modlora_set_spreading_factor);
static MP_DEFINE_CONST_FUN_OBJ_1(modlora_set_bandwidth_obj,        modlora_set_bandwidth);
static MP_DEFINE_CONST_FUN_OBJ_1(modlora_set_coding_rate_obj,      modlora_set_coding_rate);
static MP_DEFINE_CONST_FUN_OBJ_1(modlora_set_preamble_length_obj,  modlora_set_preamble_length);
static MP_DEFINE_CONST_FUN_OBJ_1(modlora_set_sync_word_obj,        modlora_set_sync_word);
static MP_DEFINE_CONST_FUN_OBJ_0(modlora_enable_crc_obj,           modlora_enable_crc);
static MP_DEFINE_CONST_FUN_OBJ_0(modlora_disable_crc_obj,          modlora_disable_crc);
static MP_DEFINE_CONST_FUN_OBJ_1(modlora_send_packet_obj,          modlora_send_packet);
static MP_DEFINE_CONST_FUN_OBJ_0(modlora_received_obj,             modlora_received);
static MP_DEFINE_CONST_FUN_OBJ_0(modlora_receive_packet_obj,       modlora_receive_packet);

static const mp_rom_map_elem_t lora_module_globals_table[] = {
	{MP_ROM_QSTR(MP_QSTR_set_header_explicit ), MP_ROM_PTR(&modlora_set_header_explicit_obj)},
	{MP_ROM_QSTR(MP_QSTR_set_header_implicit ), MP_ROM_PTR(&modlora_set_header_implicit_obj)},
	{MP_ROM_QSTR(MP_QSTR_set_mode_idle       ), MP_ROM_PTR(&modlora_set_mode_idle_obj)},
	{MP_ROM_QSTR(MP_QSTR_set_mode_sleep      ), MP_ROM_PTR(&modlora_set_mode_sleep_obj)},
	{MP_ROM_QSTR(MP_QSTR_set_mode_receive    ), MP_ROM_PTR(&modlora_set_mode_receive_obj)},
	{MP_ROM_QSTR(MP_QSTR_set_tx_power        ), MP_ROM_PTR(&modlora_set_tx_power_obj)},
	{MP_ROM_QSTR(MP_QSTR_set_frequency       ), MP_ROM_PTR(& modlora_set_frequency_obj)},
	{MP_ROM_QSTR(MP_QSTR_set_spreading_factor), MP_ROM_PTR(&modlora_set_spreading_factor_obj)},
	{MP_ROM_QSTR(MP_QSTR_set_bandwidth       ), MP_ROM_PTR(& modlora_set_bandwidth_obj)},
	{MP_ROM_QSTR(MP_QSTR_set_coding_rate     ), MP_ROM_PTR(& modlora_set_coding_rate_obj)},
	{MP_ROM_QSTR(MP_QSTR_set_preamble_length ), MP_ROM_PTR(& modlora_set_preamble_length_obj)},
	{MP_ROM_QSTR(MP_QSTR_set_sync_word       ), MP_ROM_PTR(& modlora_set_sync_word_obj)},
	{MP_ROM_QSTR(MP_QSTR_enable_crc          ), MP_ROM_PTR(& modlora_enable_crc_obj)},
	{MP_ROM_QSTR(MP_QSTR_disable_crc         ), MP_ROM_PTR(& modlora_disable_crc_obj)},
	{MP_ROM_QSTR(MP_QSTR_send_packet         ), MP_ROM_PTR(&modlora_send_packet_obj)},
	{MP_ROM_QSTR(MP_QSTR_received            ), MP_ROM_PTR(&modlora_received_obj)},
	{MP_ROM_QSTR(MP_QSTR_receive_packet      ), MP_ROM_PTR(&modlora_receive_packet_obj)},
};

static MP_DEFINE_CONST_DICT(lora_module_globals, lora_module_globals_table);

const mp_obj_module_t lora_module = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *)&lora_module_globals,
};

#endif
