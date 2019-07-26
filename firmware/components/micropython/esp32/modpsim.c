#include <sdkconfig.h>

#ifdef CONFIG_PARTICLE_SIMULATION_ENABLE

#include <string.h>

#include "esp_log.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "lib/utils/pyexec.h"

#include "extmod/vfs_native.h"

#include <compositor.h>
#include <driver_hub75.h>
#include <particle_simulation.h>

#define TAG "esp32/psim"

STATIC mp_obj_t psim_init(mp_obj_t particles_obj) {
    compositor_disable();
    if(particle_init() != ESP_OK) {
        mp_raise_msg(&mp_type_AssertionError, "MPU Not detected");
        compositor_enable();
        return mp_const_none;
    }
    int nflakes = mp_obj_get_int(particles_obj);
    particle_setBuffer(getFrameBuffer());
    particle_initSim(nflakes);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(psim_init_obj, psim_init);

STATIC mp_obj_t psim_stop() {
    particle_disable();
    compositor_enable();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(psim_stop_obj, psim_stop);

STATIC const mp_rom_map_elem_t psim_module_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_psim)},
    {MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&psim_init_obj)},
{MP_ROM_QSTR(MP_QSTR_stop), MP_ROM_PTR(&psim_stop_obj)},
};

STATIC MP_DEFINE_CONST_DICT(psim_module_globals, psim_module_globals_table);

const mp_obj_module_t psim_module = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&psim_module_globals,
};

#endif