// Include required definitions first.
#include "sdkconfig.h"
#include <time.h>
#include <string.h>
#include "py/builtin.h"
#include "py/objlist.h"
#include "py/objtuple.h"
#include "py/objstr.h"
#include "py/objint.h"
#include "py/objtype.h"
#include "py/stream.h"
#include "py/smallint.h"
#include "py/runtime.h"
#include "lib/utils/pyexec.h"

#define INT_TO_STR_EX(number) #number
#define INT_TO_STR(number) INT_TO_STR_EX(number)

STATIC const MP_DEFINE_STR_OBJ(info_firmware_name_obj, CONFIG_INFO_FIRMWARE_NAME);
STATIC const MP_DEFINE_STR_OBJ(info_firmware_build_obj, INT_TO_STR(CONFIG_INFO_FIRMWARE_BUILD));

STATIC const MP_DEFINE_STR_OBJ(info_hardware_name_obj, CONFIG_INFO_HARDWARE_NAME);
STATIC const MP_DEFINE_STR_OBJ(info_hardware_folder_obj, CONFIG_INFO_HARDWARE_FOLDER);

STATIC const MP_DEFINE_STR_OBJ(wifi_ssid_obj, CONFIG_WIFI_SSID);
STATIC const MP_DEFINE_STR_OBJ(wifi_pass_obj, CONFIG_WIFI_PASSWORD);

STATIC const mp_rom_map_elem_t consts_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR_INFO_FIRMWARE_NAME),			MP_ROM_PTR(&info_firmware_name_obj) },
    { MP_ROM_QSTR(MP_QSTR_INFO_FIRMWARE_BUILD),		    MP_ROM_PTR(&info_firmware_build_obj) },

    { MP_ROM_QSTR(MP_QSTR_INFO_HARDWARE_NAME),			MP_ROM_PTR(&info_hardware_name_obj) },
    { MP_ROM_QSTR(MP_QSTR_INFO_HARDWARE_FOLDER),		MP_ROM_PTR(&info_hardware_folder_obj) },

    { MP_ROM_QSTR(MP_QSTR_WIFI_SSID),			        MP_ROM_PTR(&wifi_ssid_obj) },
    { MP_ROM_QSTR(MP_QSTR_WIFI_PASSWORD),			    MP_ROM_PTR(&wifi_pass_obj) },
};

STATIC MP_DEFINE_CONST_DICT(consts_module_globals, consts_module_globals_table);

// Define module object.
const mp_obj_module_t consts_module = {
        .base = { &mp_type_module },
        .globals = (mp_obj_dict_t*)&consts_module_globals,
};