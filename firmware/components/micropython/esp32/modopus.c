#include <sdkconfig.h>

#ifdef CONFIG_MICROPY_USE_OPUS
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <esp_err.h>
#include <esp_log.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "py/objarray.h"
#include "py/objstr.h"

#include "opus.h"

#define TAG "opus"

typedef struct {
  mp_obj_base_t base;
  OpusEncoder *encoder;
  int channels;
  int frequency;
} libopus_encoder_obj_t;
static mp_obj_t libopus_encoder_init(const mp_obj_type_t *, size_t, size_t, const mp_obj_t *);
static mp_obj_t libopus_encode(mp_obj_t, mp_obj_t, mp_obj_t);
static const mp_obj_fun_builtin_fixed_t libopus_encode_obj;
static const mp_rom_map_elem_t libopus_encoder_locals_table[] =
  {
   {MP_OBJ_NEW_QSTR(MP_QSTR_encode), MP_ROM_PTR(&libopus_encode_obj)}
  };
STATIC MP_DEFINE_CONST_DICT(libopus_encoder_locals_dict, libopus_encoder_locals_table);
const mp_obj_type_t libopus_encoder_type =
  {
   {&mp_type_type},
   .name = MP_QSTR_Encoder,
   .make_new = libopus_encoder_init,
   .locals_dict = (mp_obj_t)&libopus_encoder_locals_dict
  };

typedef struct {
  mp_obj_base_t base;
  OpusDecoder *decoder;
  int channels;
  int frequency;
} libopus_decoder_obj_t;
static mp_obj_t libopus_decoder_init(const mp_obj_type_t *, size_t, size_t, const mp_obj_t *);
static mp_obj_t libopus_decode(mp_uint_t, const mp_obj_t *);
static const mp_obj_fun_builtin_var_t libopus_decode_obj;
static const mp_rom_map_elem_t libopus_decoder_locals_table[] =
  {
   {MP_OBJ_NEW_QSTR(MP_QSTR_decode), MP_ROM_PTR(&libopus_decode_obj)}
  };
STATIC MP_DEFINE_CONST_DICT(libopus_decoder_locals_dict, libopus_decoder_locals_table);
const mp_obj_type_t libopus_decoder_type =
  {
   {&mp_type_type},
   .name = MP_QSTR_Decoder,
   .make_new = libopus_decoder_init,
   .locals_dict = (mp_obj_t)&libopus_decoder_locals_dict
  };

static const mp_rom_map_elem_t libopus_globals_table[] =
  {
   {MP_OBJ_NEW_QSTR(MP_QSTR_Encoder), MP_ROM_PTR(&libopus_encoder_type)},
   {MP_OBJ_NEW_QSTR(MP_QSTR_Decoder), MP_ROM_PTR(&libopus_decoder_type)},
  };
static MP_DEFINE_CONST_DICT(libopus_globals, libopus_globals_table);

static mp_obj_t libopus_encoder_init(const mp_obj_type_t *type,
                                     size_t argc, size_t kwc, const mp_obj_t *argv) {
  enum {
        ARG_freq, ARG_stereo
  };
  static const mp_arg_t libopus_encoder_allowed_args[] =
    {
     { MP_QSTR_freq, MP_ARG_INT, {.u_int = 0} },
     { MP_QSTR_stereo, MP_ARG_INT, {.u_int = 0} }
    };
  mp_arg_val_t args[MP_ARRAY_SIZE(libopus_encoder_allowed_args)];
  mp_arg_parse_all_kw_array(argc, kwc, argv, MP_ARRAY_SIZE(libopus_encoder_allowed_args), libopus_encoder_allowed_args, args);
  int frequency = args[ARG_freq].u_int;
  int stereo = args[ARG_stereo].u_int;
  if(frequency == 0) {
    mp_raise_ValueError("Invalid frequency, must be 8, 12, 16, 24, or 48 kHz (in Hz)");
  }

  libopus_encoder_obj_t *self = m_new_obj_with_finaliser(libopus_encoder_obj_t);
  int err = 0;
  self->base.type = &libopus_encoder_type;
  self->encoder = (OpusEncoder*) m_new(uint8_t, opus_encoder_get_size(1 + !!stereo));
  self->channels = 1 + !!stereo;
  self->frequency = frequency;
  err = opus_encoder_init(self->encoder, frequency, stereo ? 2 : 1, OPUS_APPLICATION_VOIP);
  if(err != OPUS_OK) {
    m_free(self->encoder);
    mp_raise_msg(&mp_type_RuntimeError, "Failed to initialize encoder");
  }

  return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t libopus_encode(mp_obj_t _self, mp_obj_t _input, mp_obj_t output_or_size) {
  libopus_encoder_obj_t *self = MP_OBJ_TO_PTR(_self);
  OpusEncoder *enc = self->encoder;

  const void *input = NULL;
  size_t input_len = 0;

  if(MP_OBJ_IS_STR_OR_BYTES(_input)) {
    input = mp_obj_str_get_data(_input, &input_len);
  } else if(MP_OBJ_IS_TYPE(_input, &mp_type_bytearray)) {
    mp_obj_array_t const *input_array = MP_OBJ_TO_PTR(_input);
    input = input_array->items;
    input_len = input_array->len;
  } else {
    mp_raise_ValueError("Input needs to be bytes or bytearray");
  }

  mp_obj_t output;
  mp_obj_array_t *output_array = NULL;
  void *output_data = NULL;
  size_t output_len = 0;

  if(MP_OBJ_IS_INT(output_or_size) && mp_obj_get_int(output_or_size) > 0) {
    output_len = mp_obj_get_int(output_or_size);
    output_data = m_malloc(output_len);
    output = mp_obj_new_bytearray(output_len, output_data);
  } else if(MP_OBJ_IS_TYPE(output_or_size, &mp_type_bytearray)) {
    output = output_or_size;
  } else {
    ESP_LOGE(TAG, "Output must be a positive int or bytearray");
    return mp_const_none;
  }
  output_array = MP_OBJ_TO_PTR(output);
  output_data = output_array->items;
  output_len = output_array->len + output_array->free;
  ((uint8_t*)output_data)[0] = self->channels;
  ((uint8_t*)output_data)[1] = self->frequency / 400;
  int ret = opus_encode(enc,
                        input, input_len / sizeof(int16_t) / self->channels,
                        &((uint8_t*)output_data)[4], output_len - 4);
  if(ret >= 0) {
    ((uint16_t*)output_data)[1] = ret;
    output_array->items = m_realloc(output_data, ret + 4);
    output_array->len = ret + 4;
    output_array->free = 0;
  } else {
    ESP_LOGE(TAG, "encoding failed with error %d", -ret);
    output_array->len = 0;
    output_array->free = output_len;
  }

  return output;
}
static MP_DEFINE_CONST_FUN_OBJ_3          (libopus_encode_obj,             libopus_encode );

static mp_obj_t libopus_decoder_init(const mp_obj_type_t *type,
                                     size_t argc, size_t kwc, const mp_obj_t *argv) {
  static const mp_arg_t libopus_decoder_allowed_args[] =
    {
    };
  mp_arg_val_t args[MP_ARRAY_SIZE(libopus_decoder_allowed_args)];
  mp_arg_parse_all_kw_array(argc, kwc, argv, MP_ARRAY_SIZE(libopus_decoder_allowed_args), libopus_decoder_allowed_args, args);

  libopus_decoder_obj_t *self = m_new_obj_with_finaliser(libopus_decoder_obj_t);
  int err = 0;
  self->base.type = &libopus_decoder_type;
  self->decoder = (OpusDecoder*) m_new(uint8_t, opus_encoder_get_size(2));
  self->channels = 0;
  self->frequency = 0;
  err = opus_decoder_init(self->decoder, 8000, 1);
  if(err != OPUS_OK) {
    m_free(self->decoder);
    mp_raise_msg(&mp_type_RuntimeError, "Failed to initialize decoder");
  }

  return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t libopus_decode(mp_uint_t argc, const mp_obj_t *argv) {
  libopus_decoder_obj_t *self = MP_OBJ_TO_PTR(argv[0]);
  OpusDecoder *dec = self->decoder;

  mp_obj_t _input = argv[1];
  const void *input = NULL;
  size_t input_len = 0;

  if(MP_OBJ_IS_STR_OR_BYTES(_input)) {
    input = mp_obj_str_get_data(_input, &input_len);
  } else if(MP_OBJ_IS_TYPE(_input, &mp_type_bytearray)) {
    mp_obj_array_t const *input_array = MP_OBJ_TO_PTR(_input);
    input = input_array->items;
    input_len = input_array->len;
  } else {
    mp_raise_ValueError("Input needs to be bytes or bytearray");
  }

  mp_obj_t output;
  mp_obj_array_t *output_array = NULL;
  void *output_data = NULL;
  size_t output_len = 0;

  int channels = ((uint8_t*)input)[0];
  int frequency = 400 * ((uint8_t*)input)[1];
  int input_size_needed = ((uint16_t*)input)[1];
  if(input_size_needed > input_len - 4) {
    mp_raise_ValueError("Input packet truncated");
  }
  if(channels != self->channels || frequency != self->frequency) {
    self->channels = channels;
    self->frequency = frequency;
    opus_decoder_init(self->decoder, frequency, channels);
  }
  input = ((char*)input) + 4;
  input_len -= 4;

  if(argc > 2) {
    output = argv[2];
    if(!MP_OBJ_IS_TYPE(output, &mp_type_bytearray)) {
      mp_raise_ValueError("Output needs to be a bytearray");
    }
    output_array = MP_OBJ_TO_PTR(output);
  } else {
    output_len = opus_packet_get_nb_samples(input, input_len, self->frequency) * sizeof(int16_t) * self->channels;
    output_data = m_malloc(output_len);
    output = mp_obj_new_bytearray(output_len, output_data);
    output_array = MP_OBJ_TO_PTR(output);
  }

  output_data = output_array->items;
  output_len = output_array->len + output_array->free;
  int ret = opus_decode(dec, input, input_len, output_data, output_len / self->channels / sizeof(int16_t), 0);
  if(ret >= 0) {
    int bytes = ret * self->channels * sizeof(int16_t);
    output_array->len = bytes;
    output_array->free = output_len - bytes;
  } else {
    ESP_LOGE(TAG, "decoding failed with error %d", -ret);
    output_array->len = 0;
    output_array->free = output_len;
  }

  return output;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(libopus_decode_obj, 2, 3,       libopus_decode );

const mp_obj_module_t libopus_module = {
  .base = {&mp_type_module},
  .globals = (mp_obj_dict_t *)&libopus_globals,
};

#endif
