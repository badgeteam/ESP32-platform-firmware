#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/objarray.h"

#include "driver_microphone.h"

#ifdef CONFIG_DRIVER_MICROPHONE_ENABLE

static mp_obj_t microphone_enable(mp_uint_t n_args, const mp_obj_t *args) {
  int ms = 60;
  if(n_args > 0) {
    ms = mp_obj_get_int(args[0]);
  }
  if(ms != 10 && ms != 20 && ms != 40 && ms != 60) {
    printf("Microphone frame size must be 10, 20, 40, or 60 milliseconds\r\n");
    return mp_obj_new_int(ESP_ERR_INVALID_ARG);
  }
  int frame_size = 8*ms;
  return mp_obj_new_int(driver_microphone_start(MIC_SAMP_RATE_8_KHZ, frame_size, 250));
}

static mp_obj_t microphone_disable() {
    driver_microphone_stop();
  return mp_obj_new_int(1);
}

static mp_obj_t microphone_read(mp_uint_t n_args, const mp_obj_t *args) {
  if(n_args == 0 && driver_microphone_running()) {
    mic_data data;
    while(ESP_OK != driver_microphone_ring_buffer_get(&data)) {
      ;
    }
    mp_obj_t bytes = mp_obj_new_bytearray(data.data_size, data.data);
    free(data.data);
    mp_obj_t rv = mp_obj_new_list(1, &bytes);
    return rv;
  } else {
    if(MP_OBJ_IS_INT(args[0])) {
      mp_obj_t rv = mp_obj_new_list(0, NULL);
      int status = 0;
      int max_len = mp_obj_get_int(args[0]);
      do {
        mic_data data;
        status = driver_microphone_ring_buffer_get(&data);
        if(ESP_OK == status) {
          mp_obj_t bytes = mp_obj_new_bytearray(data.data_size, data.data);
          free(data.data);
          mp_obj_list_append(rv, bytes);
          max_len--;
        }
      } while(ESP_OK == status && max_len);
      return rv;
    } else if(MP_OBJ_IS_TYPE(args[0], &mp_type_bytearray)) {
      size_t reqd = sizeof(size_t) + driver_microphone_ring_buffer_peek_size();
      mp_obj_array_t *dest = MP_OBJ_TO_PTR(args[1]);
      size_t total = dest->len + dest->free;
      if(total >= driver_microphone_ring_buffer_peek_size()) {
        mic_data md;
        driver_microphone_ring_buffer_get(&md);
        memcpy(dest->items, md.data, md.data_size);
        dest->len = reqd;
        dest->free = total - dest->len;
        free(md.data);
        return args[1];
      } else {
        printf("Microphone: Buffer of length %d too short for %d bytes\r\n", total, reqd);
      }
    }
  }
  return mp_const_none;
}

static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(microphone_enable_obj,  0, 1, microphone_enable  );
static MP_DEFINE_CONST_FUN_OBJ_0          (microphone_disable_obj,       microphone_disable );
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(microphone_read_obj,    0, 1, microphone_read    );

static const mp_rom_map_elem_t microphone_module_globals_table[] = {
  {MP_OBJ_NEW_QSTR(MP_QSTR_enable), (mp_obj_t)&microphone_enable_obj},
  {MP_OBJ_NEW_QSTR(MP_QSTR_disable), (mp_obj_t)&microphone_disable_obj},
  {MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&microphone_read_obj},
};

static MP_DEFINE_CONST_DICT(microphone_module_globals, microphone_module_globals_table);

const mp_obj_module_t microphone_module = {
  .base = {&mp_type_module},
  .globals = (mp_obj_dict_t *)&microphone_module_globals,
};


#endif
