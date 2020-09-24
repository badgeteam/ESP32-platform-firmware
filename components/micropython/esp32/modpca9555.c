#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "py/obj.h"

#include <driver_pca9555.h>

#ifdef CONFIG_DRIVER_PCA9555_ENABLE

/* Input interrupt handling */

static mp_obj_t button_callbacks[16] = {
	mp_const_none, mp_const_none, mp_const_none, mp_const_none,
	mp_const_none, mp_const_none, mp_const_none, mp_const_none,
	mp_const_none, mp_const_none, mp_const_none, mp_const_none,
	mp_const_none, mp_const_none, mp_const_none, mp_const_none,
};

static void pca9555_event_handler(uint8_t pin, bool state)
{
	if (pin > 15) return;
	if(button_callbacks[pin] != mp_const_none) {
		if ((MP_OBJ_IS_FUN(button_callbacks[pin])) || (MP_OBJ_IS_METH(button_callbacks[pin]))) {
			mp_sched_schedule(button_callbacks[pin], mp_obj_new_bool(state), NULL);
		}
	}
}

/* Public API functions */

static mp_obj_t pca9555_attach(mp_obj_t _pin, mp_obj_t _func) {
	int pin = mp_obj_get_int(_pin);
	if ((pin < 0) || (pin > 15)) {
		mp_raise_ValueError("pin number out of range (0-15)");
		return mp_const_none;
	}
	driver_pca9555_set_interrupt_handler(pin, pca9555_event_handler);
	if ((!MP_OBJ_IS_FUN(_func) && (!MP_OBJ_IS_METH(_func)))) {
		mp_raise_ValueError("expected callback to be callable");
		return mp_const_none;
	}
	button_callbacks[pin] = _func;
	return mp_const_none;
}


static mp_obj_t pca9555_detach(mp_obj_t _pin) {
	int pin = mp_obj_get_int(_pin);
	if ((pin < 0) || (pin > 15)) return mp_const_none;
	button_callbacks[pin] = mp_const_none;
	return mp_const_none;
}

static mp_obj_t pca9555_direction(mp_uint_t n_args, const mp_obj_t *args) {
	int pin = mp_obj_get_int(args[0]);
	if (n_args == 2) { //Set
		bool state = mp_obj_get_int(args[1]);
		int res = driver_pca9555_set_gpio_direction(pin, state);
		if (res < 0) mp_raise_ValueError("unable to change pin direction for this pin");
		return mp_const_none;
	} else { //Get
		int res = driver_pca9555_get_gpio_direction(pin);
		if (res < 0) {
			mp_raise_ValueError("unable to read pin direction for this pin");
			return mp_const_none;
		}
		return mp_obj_new_bool(res);
	}
}

static mp_obj_t pca9555_value(mp_uint_t n_args, const mp_obj_t *args) {
	int pin = mp_obj_get_int(args[0]);
	if (n_args == 2) { //Set
		bool state = mp_obj_get_int(args[1]);
		int res = driver_pca9555_set_gpio_value(pin, state);
		if (res < 0) mp_raise_ValueError("unable to change pin value for this pin");
		return mp_const_none;
	} else { //Get
		int res = driver_pca9555_get_gpio_value(pin);
		if (res < 0) {
			mp_raise_ValueError("unable to read pin value for this pin");
			return mp_const_none;
		}
		return mp_obj_new_bool(res);
	}
}

static mp_obj_t pca9555_polarity(mp_uint_t n_args, const mp_obj_t *args) {
	int pin = mp_obj_get_int(args[0]);
	if (n_args == 2) { //Set
		bool state = mp_obj_get_int(args[1]);
		int res = driver_pca9555_set_gpio_polarity(pin, state);
		if (res < 0) mp_raise_ValueError("unable to change pin polarity for this pin");
		return mp_const_none;
	} else { //Get
		int res = driver_pca9555_get_gpio_polarity(pin);
		if (res < 0) {
			mp_raise_ValueError("unable to read pin polarity for this pin");
			return mp_const_none;
		}
		return mp_obj_new_bool(res);
	}
}

static MP_DEFINE_CONST_FUN_OBJ_2          ( pca9555_attach_obj,          pca9555_attach    );
static MP_DEFINE_CONST_FUN_OBJ_1          ( pca9555_detach_obj,          pca9555_detach    );
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( pca9555_direction_obj, 1, 2, pca9555_direction );
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( pca9555_value_obj,     1, 2, pca9555_value     );
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( pca9555_polarity_obj,  1, 2, pca9555_polarity  );

static const mp_rom_map_elem_t pca9555_module_globals_table[] = {
	{MP_ROM_QSTR(MP_QSTR_attach),    MP_ROM_PTR(&pca9555_attach_obj)},    //pca9555.attach(pin, func)
	{MP_ROM_QSTR(MP_QSTR_detach),    MP_ROM_PTR(&pca9555_detach_obj)},    //pca9555.detach(pin)
	{MP_ROM_QSTR(MP_QSTR_direction), MP_ROM_PTR(&pca9555_direction_obj)}, //pca9555.direction(pin) and pca9555.direction(pin, direction)
	{MP_ROM_QSTR(MP_QSTR_value),     MP_ROM_PTR(&pca9555_value_obj)},     //pca9555.value(pin) and pca9555.value(pin, value)
	{MP_ROM_QSTR(MP_QSTR_polarity),  MP_ROM_PTR(&pca9555_polarity_obj)},  //pca9555.polarity(pin) and pca9555.polarity(pin, polarity)
};

static MP_DEFINE_CONST_DICT(pca9555_module_globals, pca9555_module_globals_table);

const mp_obj_module_t pca9555_module = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *) &pca9555_module_globals,
};

#endif // CONFIG_DRIVER_I2C_ENABLE
