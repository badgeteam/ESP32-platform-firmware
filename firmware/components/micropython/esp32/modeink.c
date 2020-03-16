#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include <driver_eink.h>
#include <driver_eink_dev.h>

#ifdef CONFIG_DRIVER_EINK_ENABLE

static mp_obj_t eink_deep_sleep()
{
	driver_eink_deep_sleep();
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(eink_deep_sleep_obj, eink_deep_sleep);

static mp_obj_t eink_wakeup()
{
	eink_wakeup();
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(eink_wakeup_obj, eink_wakeup);

static mp_obj_t eink_busy()
{
	return mp_obj_new_bool(driver_eink_dev_is_busy());
}
static MP_DEFINE_CONST_FUN_OBJ_0(eink_busy_obj, eink_busy);

static mp_obj_t eink_busy_wait() {
	driver_eink_dev_busy_wait();
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(eink_busy_wait_obj, eink_busy_wait);

static mp_obj_t eink_display_raw(mp_obj_t obj_img, mp_obj_t obj_flags)
{
	bool is_bytes = MP_OBJ_IS_TYPE(obj_img, &mp_type_bytes);

	if (!is_bytes) {
		mp_raise_msg(&mp_type_AttributeError, "First argument should be a bytestring");
	}

	// convert the input buffer into a byte array
	mp_uint_t len;
	uint8_t *buffer = (uint8_t *)mp_obj_str_get_data(obj_img, &len);

	int flags = mp_obj_get_int(obj_flags);
	int expect_len = (flags & DISPLAY_FLAG_8BITPIXEL) ? DRIVER_EINK_WIDTH*DRIVER_EINK_HEIGHT : DRIVER_EINK_WIDTH*DRIVER_EINK_HEIGHT/8;
	if (len != expect_len) {
		mp_raise_msg(&mp_type_AttributeError, "First argument has wrong length");
	}

	// display the image directly
	driver_eink_display(buffer, flags);

	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(eink_display_raw_obj, eink_display_raw);


static mp_obj_t eink_set_minimal_update_height(mp_obj_t height_obj)
{
	driver_eink_set_minimal_update_height(mp_obj_get_int(height_obj));
	return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_1(eink_set_minimal_update_height_obj, eink_set_minimal_update_height);

static const mp_rom_map_elem_t eink_module_globals_table[] = {
	{MP_ROM_QSTR(MP_QSTR_deep_sleep), MP_ROM_PTR(&eink_deep_sleep_obj)},
	{MP_ROM_QSTR(MP_QSTR_wakeup), MP_ROM_PTR(&eink_wakeup_obj)},
	{MP_ROM_QSTR(MP_QSTR_busy), MP_ROM_PTR(&eink_busy_obj)},
	{MP_ROM_QSTR(MP_QSTR_busy_wait), MP_ROM_PTR(&eink_busy_wait_obj)},
	{MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&eink_display_raw_obj)},
	{MP_ROM_QSTR(MP_QSTR_setMinimalUpdateHeight), MP_ROM_PTR(&eink_set_minimal_update_height_obj)},
};

static MP_DEFINE_CONST_DICT(eink_module_globals, eink_module_globals_table);

const mp_obj_module_t eink_module = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *)&eink_module_globals,
};

#endif // CONFIG_DRIVER_EINK_ENABLE
