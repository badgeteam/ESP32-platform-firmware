#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include <driver_apa102.h>

#ifdef CONFIG_DRIVER_APA102_ENABLE

static mp_obj_t send(mp_uint_t n_args, const mp_obj_t *args) {
    if (!MP_OBJ_IS_TYPE(args[0], &mp_type_bytes)) {
        mp_raise_ValueError("Expected a bytestring like object.");
    } else {
        mp_uint_t length;
        uint8_t *data = (uint8_t *)mp_obj_str_get_data(args[0], &length);
        if (driver_apa102_send_data(data, length) != ESP_OK) {
            mp_raise_ValueError("failed to send data");
        }
    }
    return mp_const_none;
}

static mp_obj_t brightness(mp_uint_t n_args, const mp_obj_t *args) {
    driver_apa102_set_brightness(mp_obj_get_int(args[0]));
    return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(send_obj, 1, 1, send );
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(brightness_obj, 1, 1, brightness );

static const mp_rom_map_elem_t apa102_module_globals_table[] = {
    {MP_OBJ_NEW_QSTR(MP_QSTR_send), (mp_obj_t)&send_obj},
    {MP_OBJ_NEW_QSTR(MP_QSTR_brightness), (mp_obj_t)&brightness_obj},
};

static MP_DEFINE_CONST_DICT(apa102_module_globals, apa102_module_globals_table);

const mp_obj_module_t apa102_module = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&apa102_module_globals,
};

#endif
