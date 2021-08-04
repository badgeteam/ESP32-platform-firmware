#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "py/obj.h"

#include <driver_mch2021_stm32.h>

#ifdef CONFIG_DRIVER_MCH2021_STM32_ENABLE

static mp_obj_t stm32_transaction(mp_uint_t n_args, const mp_obj_t *args) {
    if (!MP_OBJ_IS_TYPE(args[0], &mp_type_bytes)) {
        mp_raise_ValueError("Expected a bytestring like object");
        return mp_const_none;
    }
    
    mp_uint_t outputLength;
    uint8_t* outputData = (uint8_t*) mp_obj_str_get_data(args[0], &outputLength);
    if (outputLength != 18) {
        mp_raise_ValueError("Expected 18 bytes");
        return mp_const_none;
    }
    
    uint8_t inputData[18];
    esp_err_t res = driver_mch2021_stm32_transaction(outputData, inputData);
    if (res != ESP_OK) {
        mp_raise_ValueError("Failed to execute transaction");
    }
    
    return mp_obj_new_bytes(inputData, 18);
}


static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN( stm32_transaction_obj, 1, 1, stm32_transaction );

static const mp_rom_map_elem_t mch2021_stm32_module_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR_transaction), MP_ROM_PTR(&stm32_transaction_obj)}, //input = stm32.transaction(output)
};

static MP_DEFINE_CONST_DICT(mch2021_stm32_module_globals, mch2021_stm32_module_globals_table);

const mp_obj_module_t mch2021_stm32_module = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *) &mch2021_stm32_module_globals,
};

#endif
