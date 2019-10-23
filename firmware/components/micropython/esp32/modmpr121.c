#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "py/obj.h"

#include <driver_mpr121.h>

#ifdef CONFIG_DRIVER_MPR121_ENABLE

static mp_obj_t mpr121_is_digital_input(mp_obj_t _pin) {
	int pin = mp_obj_get_int(_pin);
	bool result = driver_mpr121_is_digital_input(pin);
	return mp_obj_new_bool(result);
}

static mp_obj_t mpr121_is_digital_output(mp_obj_t _pin) {
	int pin = mp_obj_get_int(_pin);
	bool result = driver_mpr121_is_digital_output(pin);
	return mp_obj_new_bool(result);
}

static mp_obj_t mpr121_is_touch_input(mp_obj_t _pin) {
	int pin = mp_obj_get_int(_pin);
	bool result = driver_mpr121_is_touch_input(pin);
	return mp_obj_new_bool(result);
}

static mp_obj_t mpr121_set_digital_output(mp_obj_t _pin, mp_obj_t _value) {
	int pin = mp_obj_get_int(_pin);
	bool value = mp_obj_get_int(_value);
	if (!driver_mpr121_is_digital_output(pin)) {
		return mp_obj_new_bool(false);
	} else {
		esp_err_t result = driver_mpr121_set_gpio_level(pin, value);
		return mp_obj_new_bool(result == ESP_OK);
	}
}

/* Input handling */

static mp_obj_t button_callbacks[12] = {
	mp_const_none, mp_const_none, mp_const_none,
	mp_const_none, mp_const_none, mp_const_none,
	mp_const_none, mp_const_none, mp_const_none,
	mp_const_none, mp_const_none, mp_const_none
};

static void mpr121_event_handler(void *b, bool state)
{
	int pin = (uint32_t) b;
	if ((pin < 0) || (pin > 11)) return;
	if(button_callbacks[pin] != mp_const_none){
		if ((!MP_OBJ_IS_FUN(button_callbacks[pin])) && (!MP_OBJ_IS_METH(button_callbacks[pin]))) {
			printf("MPR121 ERROR: CALLBACK IS NOT FUNCTION OR METHOD?!?! (pin %u)\n", pin);
		} else {
			mp_sched_schedule(button_callbacks[pin], mp_obj_new_bool(state), NULL);
		}
	}
}

static mp_obj_t mpr121_input_attach(mp_obj_t _pin, mp_obj_t _func) {
	int pin = mp_obj_get_int(_pin);
	if ((pin < 0) || (pin > 11)) return mp_const_none;
	driver_mpr121_set_interrupt_handler(pin, mpr121_event_handler, (void*) (pin));
	if ((!MP_OBJ_IS_FUN(_func) && (!MP_OBJ_IS_METH(_func)))) {
		mp_raise_ValueError("callback function expected");
		return mp_const_none;
	}
	button_callbacks[pin] = _func;
	return mp_const_none;
}


static mp_obj_t mpr121_input_detach(mp_obj_t _pin) {
  int pin = mp_obj_get_int(_pin);
  if ((pin < 0) || (pin > 11)) return mp_const_none;
  button_callbacks[pin] = mp_const_none;
  return mp_const_none;
}

/* -------------- */

static mp_obj_t mpr121_input_read(mp_obj_t _pin) {
  int pin = mp_obj_get_int(_pin);
  if ((pin < 0) || (pin > 11)) return mp_const_none;
  if (driver_mpr121_is_digital_input(pin) || driver_mpr121_is_digital_output(pin)) return mp_obj_new_bool(driver_mpr121_get_gpio_level(pin));
  if (driver_mpr121_is_touch_input(pin)) return mp_obj_new_bool(driver_mpr121_get_touch_level(pin));
  return mp_const_none;
}

#define store_dict_int(dict, field, contents) mp_obj_dict_store(dict, mp_obj_new_str(field, strlen(field)), mp_obj_new_int(contents));
static mp_obj_t mpr121_get_touch_info(void) {
	struct driver_mpr121_touch_info info;
	esp_err_t err = driver_mpr121_get_touch_info(&info);
	if (err != ESP_OK) {
		mp_raise_OSError(MP_EIO);
	}

	mp_obj_t list_items[12];
	int i;
	for (i=0; i<12; i++) {
		list_items[i] = mp_obj_new_dict(0);

		mp_obj_dict_t *dict = MP_OBJ_TO_PTR(list_items[i]);

		store_dict_int(dict, "data",     info.data[i]);
		store_dict_int(dict, "baseline", info.baseline[i]);
		store_dict_int(dict, "touch",    info.touch[i]);
		store_dict_int(dict, "release",  info.release[i]);
	}
	mp_obj_t list = mp_obj_new_list(12, list_items);
	return list;
}

static mp_obj_t mpr121_configure(mp_uint_t n_args, const mp_obj_t *args)
{
	if (n_args > 0) {
		uint8_t press = 12;
		uint8_t release = 6;
		if (n_args == 2) {
			mp_raise_ValueError("Expected either 0 (baseline-tracking), 1 (static) or 3 (static + thresholds) parameters!");
			return mp_const_none;
		}
		
		if (n_args > 2) {
			press = mp_obj_get_int(args[1]);
			release = mp_obj_get_int(args[2]);
		}
		mp_uint_t len;
		mp_obj_t *items;
		mp_obj_list_get(args[0], &len, &items);
		uint32_t baseline[12];
		for (uint8_t i = 0; i < 12; i++) {
			if (len > i) {
				baseline[i] = mp_obj_get_int(items[i]);
			} else {
				baseline[i] = 0x0000;
			}
		}
		if (len < 1) {
			driver_mpr121_configure(NULL, press, release);
		} else {
			driver_mpr121_configure(baseline, press, release);
		}
	} else {	
		driver_mpr121_configure(NULL, 12, 6);
	}
	return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_1          ( mpr121_is_digital_input_obj,           mpr121_is_digital_input     );
static MP_DEFINE_CONST_FUN_OBJ_1          ( mpr121_is_digital_output_obj,          mpr121_is_digital_output    );
static MP_DEFINE_CONST_FUN_OBJ_1          ( mpr121_is_touch_input_obj,             mpr121_is_touch_input       );
static MP_DEFINE_CONST_FUN_OBJ_2          ( mpr121_set_digital_output_obj,         mpr121_set_digital_output   );
static MP_DEFINE_CONST_FUN_OBJ_2          ( mpr121_input_attach_obj,               mpr121_input_attach         );
static MP_DEFINE_CONST_FUN_OBJ_1          ( mpr121_input_detach_obj,               mpr121_input_detach         );
static MP_DEFINE_CONST_FUN_OBJ_1          ( mpr121_input_read_obj,                 mpr121_input_read           );
static MP_DEFINE_CONST_FUN_OBJ_0          ( mpr121_get_touch_info_obj,             mpr121_get_touch_info       );
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( mpr121_configure_obj,            0, 3, mpr121_configure            );

static const mp_rom_map_elem_t mpr121_module_globals_table[] = {	
	{MP_ROM_QSTR(MP_QSTR_isInput), MP_ROM_PTR(&mpr121_is_digital_input_obj)},                //mpr121.isInput(pin)
	{MP_ROM_QSTR(MP_QSTR_isOutput), MP_ROM_PTR(&mpr121_is_digital_output_obj)},              //mpr121.isOutput(pin)
	{MP_ROM_QSTR(MP_QSTR_isTouch), MP_ROM_PTR(&mpr121_is_touch_input_obj)},                  //mpr121.isTouch(pin)
	{MP_ROM_QSTR(MP_QSTR_attach), MP_ROM_PTR(&mpr121_input_attach_obj)},                     //mpr121.attach(pin, func)
	{MP_ROM_QSTR(MP_QSTR_detach), MP_ROM_PTR(&mpr121_input_detach_obj)},                     //mpr121.detach(pin)
	{MP_ROM_QSTR(MP_QSTR_set), MP_ROM_PTR(&mpr121_set_digital_output_obj)},                  //mpr121.set(pin, value)
	{MP_ROM_QSTR(MP_QSTR_get), MP_ROM_PTR(&mpr121_input_read_obj)},                          //mpr121.get(pin)
	{MP_ROM_QSTR(MP_QSTR_touchInfo), MP_ROM_PTR(&mpr121_get_touch_info_obj)},                //mpr121.touchInfo()
	{MP_ROM_QSTR(MP_QSTR_configure), MP_ROM_PTR(&mpr121_configure_obj)},                     //mpr121.configure([baseline])
};

static MP_DEFINE_CONST_DICT(mpr121_module_globals, mpr121_module_globals_table);

const mp_obj_module_t mpr121_module = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *) &mpr121_module_globals,
};

#endif // CONFIG_DRIVER_I2C_ENABLE
