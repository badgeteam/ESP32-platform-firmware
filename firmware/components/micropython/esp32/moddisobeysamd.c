#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include <driver_disobey_samd.h>

#ifdef CONFIG_DRIVER_DISOBEY_SAMD_ENABLE

STATIC mp_obj_t samd_backlight(mp_uint_t n_args, const mp_obj_t *args) {
  driver_disobey_samd_write_backlight(mp_obj_get_int(args[0]));
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(samd_backlight_obj, 1, 1, samd_backlight);

STATIC mp_obj_t samd_led(mp_uint_t n_args, const mp_obj_t *args) {
  driver_disobey_samd_write_led(mp_obj_get_int(args[0]),mp_obj_get_int(args[1]),mp_obj_get_int(args[2]),mp_obj_get_int(args[3]));
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(samd_led_obj, 4, 4, samd_led);

STATIC mp_obj_t samd_buzzer(mp_uint_t n_args, const mp_obj_t *args) {
  driver_disobey_samd_write_buzzer(mp_obj_get_int(args[0]),mp_obj_get_int(args[1]));
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(samd_buzzer_obj, 2, 2, samd_buzzer);

STATIC mp_obj_t samd_off() {
  driver_disobey_samd_write_off();
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(samd_off_obj, samd_off);

STATIC mp_obj_t samd_read_usb() {
  return mp_obj_new_int(driver_disobey_samd_read_usb());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(samd_read_usb_obj, samd_read_usb);

STATIC mp_obj_t samd_read_battery() {
  return mp_obj_new_int(driver_disobey_samd_read_battery());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(samd_read_battery_obj, samd_read_battery);

STATIC mp_obj_t samd_read_touch() {
  return mp_obj_new_int(driver_disobey_samd_read_touch());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(samd_read_touch_obj, samd_read_touch);

STATIC mp_obj_t samd_read_state() {
  return mp_obj_new_int(driver_disobey_samd_read_state());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(samd_read_state_obj, samd_read_state);

STATIC const mp_rom_map_elem_t samd_module_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_samd)},
    {MP_OBJ_NEW_QSTR(MP_QSTR_backlight), (mp_obj_t)&samd_backlight_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_led), (mp_obj_t)&samd_led_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_buzzer), (mp_obj_t)&samd_buzzer_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_off), (mp_obj_t)&samd_off_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_read_usb), (mp_obj_t)&samd_read_usb_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_read_battery), (mp_obj_t)&samd_read_battery_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_read_touch), (mp_obj_t)&samd_read_touch_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_read_state), (mp_obj_t)&samd_read_state_obj},
    };

static MP_DEFINE_CONST_DICT(samd_module_globals, samd_module_globals_table);

const mp_obj_module_t samd_module = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *)&samd_module_globals,
};

#endif // CONFIG_DRIVER_DISOBEY_SAMD_ENABLE
