//#include <sdkconfig.h>
#include "../../../build/include/sdkconfig.h"

#ifdef CONFIG_DRIVER_IO_TOUCHBUTTONS_ENABLE

#include <string.h>

#include "esp_log.h"

#include "py/obj.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "py/runtime.h"
#include "lib/utils/pyexec.h"

#include <driver_touchbuttons.h>

#define TAG "esp32/touchbuttons"

static mp_obj_t touch_callback = mp_const_none;

static void touch_handler(uint16_t touch_state) {
    if (touch_callback != mp_const_none) {
        // uPy scheduler tends to get full for unknown reasons, so to not lose any interrupts,
        // we try until the schedule succeeds.
        bool succeeded = mp_sched_schedule(touch_callback, mp_obj_new_int(touch_state), NULL);
        while (!succeeded) {
            ESP_LOGW(TAG, "Failed to call touch callback, retrying");
            succeeded = mp_sched_schedule(touch_callback, mp_obj_new_int(touch_state), NULL);
        }
    }
}

static mp_obj_t touchbuttons_set_handler(mp_obj_t handler) {
    touch_callback = handler;
    set_touch_handler(&touch_handler);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(touchbuttons_set_handler_obj, touchbuttons_set_handler);

static mp_obj_t touchbuttons_get_state() {
    uint16_t state = get_touch_state();
    return mp_obj_new_int(state);
}
static MP_DEFINE_CONST_FUN_OBJ_1(touchbuttons_get_state_obj, touchbuttons_get_state);

static mp_obj_t touchbuttons_get_value(mp_obj_t pad) {
    uint8_t cast_pad = (uint8_t) mp_obj_get_int(pad);
    uint16_t value = get_touch_value(cast_pad);
    return mp_obj_new_int(value);
}
static MP_DEFINE_CONST_FUN_OBJ_1(touchbuttons_get_value_obj, touchbuttons_get_value);

static const mp_rom_map_elem_t touchbuttons_module_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_touchbuttons)},
    {MP_ROM_QSTR(MP_QSTR_get_state), MP_ROM_PTR(&touchbuttons_get_state_obj)},
    {MP_ROM_QSTR(MP_QSTR_get_value), MP_ROM_PTR(&touchbuttons_get_value_obj)},
    {MP_ROM_QSTR(MP_QSTR_set_handler), MP_ROM_PTR(&touchbuttons_set_handler_obj)},
};

static MP_DEFINE_CONST_DICT(touchbuttons_module_globals, touchbuttons_module_globals_table);

const mp_obj_module_t touchbuttons_module = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&touchbuttons_module_globals,
};

#endif
