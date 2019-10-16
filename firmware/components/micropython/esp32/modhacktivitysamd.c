#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include <driver_hacktivity_samd.h>

#ifdef CONFIG_DRIVER_HACKTIVITY_SAMD_ENABLE

static mp_obj_t samd_backlight(mp_uint_t n_args, const mp_obj_t *args) {
  driver_hacktivity_samd_write_backlight(mp_obj_get_int(args[0]));
  return mp_const_none;
}

static mp_obj_t samd_led(mp_uint_t n_args, const mp_obj_t *args) {
  driver_hacktivity_samd_write_led(mp_obj_get_int(args[0]),mp_obj_get_int(args[1]),mp_obj_get_int(args[2]),mp_obj_get_int(args[3]));
  return mp_const_none;
}

static mp_obj_t samd_buzzer(mp_uint_t n_args, const mp_obj_t *args) {
  driver_hacktivity_samd_write_buzzer(mp_obj_get_int(args[0]),mp_obj_get_int(args[1]));
  return mp_const_none;
}

static mp_obj_t samd_off() {
  driver_hacktivity_samd_write_off();
  return mp_const_none;
}

static mp_obj_t samd_read_usb() {
  return mp_obj_new_int(driver_hacktivity_samd_read_usb());
}

static mp_obj_t samd_read_battery() {
  return mp_obj_new_int(driver_hacktivity_samd_read_battery());
}

static mp_obj_t samd_read_touch() {
  return mp_obj_new_int(driver_hacktivity_samd_read_touch());
}

static mp_obj_t samd_read_state() {
  return mp_obj_new_int(driver_hacktivity_samd_read_state());
}

/* Input handling */

bool handlerFuncAttached = false;

static mp_obj_t button_callbacks[6] = {
	mp_const_none, mp_const_none, mp_const_none,
	mp_const_none, mp_const_none, mp_const_none
};

static void samd_event_handler(int pressed, int released)
{
	for (uint8_t btn = 0; btn<6; btn++) {
		if ((pressed >> btn)&0x01) {
			if(button_callbacks[btn] != mp_const_none){
				if ((!MP_OBJ_IS_FUN(button_callbacks[btn])) && (!MP_OBJ_IS_METH(button_callbacks[btn]))) {
					printf("SAMD ERROR: CALLBACK IS NOT FUN OR METH?!?! (btn %u)\n", btn);
				} else {
					mp_sched_schedule(button_callbacks[btn], mp_obj_new_bool(1), NULL);
				}
			}
		}
		if ((released >> btn)&0x01) {
			if(button_callbacks[btn] != mp_const_none){
				if ((!MP_OBJ_IS_FUN(button_callbacks[btn])) && (!MP_OBJ_IS_METH(button_callbacks[btn]))) {
					printf("SAMD ERROR: CALLBACK IS NOT FUN OR METH?!?! (btn %u)\n", btn);
				} else {
					mp_sched_schedule(button_callbacks[btn], mp_obj_new_bool(0), NULL);
				}
			}
		}
	}
}

static mp_obj_t samd_input_attach(mp_obj_t _pin, mp_obj_t _func) {
	int pin = mp_obj_get_int(_pin);
	if ((pin < 0) || (pin > 5)) return mp_const_none;
	driver_hacktivity_samd_set_interrupt_handler(samd_event_handler);
	if ((!MP_OBJ_IS_FUN(_func) && (!MP_OBJ_IS_METH(_func)))) {
		mp_raise_ValueError("callback function expected");
		return mp_const_none;
	}
	button_callbacks[pin] = _func;
	return mp_const_none;
}


static mp_obj_t samd_input_detach(mp_obj_t _pin) {
  int pin = mp_obj_get_int(_pin);
  if ((pin < 0) || (pin > 11)) return mp_const_none;
  button_callbacks[pin] = mp_const_none;
  return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( samd_backlight_obj,    1, 1, samd_backlight    );
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( samd_led_obj,          4, 4, samd_led          );
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( samd_buzzer_obj,       2, 2, samd_buzzer       );
static MP_DEFINE_CONST_FUN_OBJ_0          ( samd_off_obj,                samd_off          );
static MP_DEFINE_CONST_FUN_OBJ_0          ( samd_read_usb_obj,           samd_read_usb     );
static MP_DEFINE_CONST_FUN_OBJ_0          ( samd_read_battery_obj,       samd_read_battery );
static MP_DEFINE_CONST_FUN_OBJ_0          ( samd_read_touch_obj,         samd_read_touch   );
static MP_DEFINE_CONST_FUN_OBJ_0          ( samd_read_state_obj,         samd_read_state   );
static MP_DEFINE_CONST_FUN_OBJ_2          ( samd_input_attach_obj,       samd_input_attach );
static MP_DEFINE_CONST_FUN_OBJ_1          ( samd_input_detach_obj,       samd_input_detach );

/* -------------- */

static const mp_rom_map_elem_t samd_module_globals_table[] = {
	{ MP_ROM_QSTR     ( MP_QSTR___name__     ), MP_ROM_QSTR(MP_QSTR_samd)          },
	{ MP_OBJ_NEW_QSTR ( MP_QSTR_backlight    ), (mp_obj_t)&samd_backlight_obj      }, //samd.backlight(brightness)
	{ MP_OBJ_NEW_QSTR ( MP_QSTR_led          ), (mp_obj_t)&samd_led_obj            }, //samd.led(?,?,?,?)
	{ MP_OBJ_NEW_QSTR ( MP_QSTR_buzzer       ), (mp_obj_t)&samd_buzzer_obj         }, //samd.buzzer(?,?)
	{ MP_OBJ_NEW_QSTR ( MP_QSTR_off          ), (mp_obj_t)&samd_off_obj            }, //samd.off()
	{ MP_OBJ_NEW_QSTR ( MP_QSTR_read_usb     ), (mp_obj_t)&samd_read_usb_obj       }, //samd.read_usb()
	{ MP_OBJ_NEW_QSTR ( MP_QSTR_read_battery ), (mp_obj_t)&samd_read_battery_obj   }, //samd.read_battery()
	{ MP_OBJ_NEW_QSTR ( MP_QSTR_read_touch   ), (mp_obj_t)&samd_read_touch_obj     }, //samd.read_touch()
	{ MP_OBJ_NEW_QSTR ( MP_QSTR_read_state   ), (mp_obj_t)&samd_read_state_obj     }, //samd.read_state()
	{ MP_ROM_QSTR     ( MP_QSTR_attach       ), MP_ROM_PTR(&samd_input_attach_obj) }, //samd.attach(pin, func)
	{ MP_ROM_QSTR     ( MP_QSTR_detach       ), MP_ROM_PTR(&samd_input_detach_obj) }, //samd.detach(pin)
};

static MP_DEFINE_CONST_DICT(samd_module_globals, samd_module_globals_table);

const mp_obj_module_t samd_module = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *)&samd_module_globals,
};

#endif // CONFIG_DRIVER_HACKTIVITY_SAMD_ENABLE
