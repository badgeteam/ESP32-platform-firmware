#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include <driver_am2320.h>

#ifdef CONFIG_DRIVER_AM2320_ENABLE

// This is the function which will be called from Python as am2320.get_temperature().
STATIC mp_obj_t am2320_get_temperature() {
    // convert to MicroPython object.
    return mp_obj_new_float(driver_am2320_get_temperature());
}

// Define a Python reference to the function above
STATIC MP_DEFINE_CONST_FUN_OBJ_0(am2320_get_temperature_obj, am2320_get_temperature);

// This is the function which will be called from Python as am2320.get_temperature().
STATIC mp_obj_t am2320_get_humidity() {
    // convert to MicroPython object.
    return mp_obj_new_float(driver_am2320_get_humidity());
}

// Define a Python reference to the function above
STATIC MP_DEFINE_CONST_FUN_OBJ_0(am2320_get_humidity_obj, am2320_get_humidity);

// Define all properties of the am2320 module.
STATIC const mp_rom_map_elem_t am2320_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_am2320) },
    { MP_ROM_QSTR(MP_QSTR_get_temperature), MP_ROM_PTR(&am2320_get_temperature_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_humidity), MP_ROM_PTR(&am2320_get_humidity_obj) },
};

STATIC MP_DEFINE_CONST_DICT(am2320_module_globals, am2320_module_globals_table);

// Register the module
const mp_obj_module_t am2320_module = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *)&am2320_module_globals,
};

#endif