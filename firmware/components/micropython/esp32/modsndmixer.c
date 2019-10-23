#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include "sndmixer.h"

#ifdef CONFIG_DRIVER_SNDMIXER_ENABLE

bool sndmixer_started  = 0;
int  sndmixer_channels = 0;

const char* msg_error_not_started = "sndmixer task not started!";

static mp_obj_t modsndmixer_begin(mp_obj_t _channels) {
	int channels = mp_obj_get_int(_channels);
	if (!sndmixer_started) {
		sndmixer_init(channels);
		sndmixer_started = 1;
		sndmixer_channels = channels;
	} else {
		printf("WARNING: The sndmixer task has already been started! The number of available channels (%d) can not be changed.\n", sndmixer_channels);
	}
	return mp_obj_new_int(sndmixer_channels);
}


static mp_obj_t modsndmixer_play(mp_obj_t _id) {
	if (!sndmixer_started) { mp_raise_ValueError(msg_error_not_started); return mp_const_none; }
	int id = mp_obj_get_int(_id);
	sndmixer_play(id);
	return mp_const_none;
}

static mp_obj_t modsndmixer_pause(mp_obj_t _id) {
	if (!sndmixer_started) { mp_raise_ValueError(msg_error_not_started); return mp_const_none; }
	int id = mp_obj_get_int(_id);
	sndmixer_pause(id);
	return mp_const_none;
}

static mp_obj_t modsndmixer_stop(mp_obj_t _id) {
	if (!sndmixer_started) { mp_raise_ValueError(msg_error_not_started); return mp_const_none; }
	int id = mp_obj_get_int(_id);
	sndmixer_stop(id);
	return mp_const_none;
}

static mp_obj_t modsndmixer_pause_all() {
	if (!sndmixer_started) { mp_raise_ValueError(msg_error_not_started); return mp_const_none; }
	sndmixer_pause_all();
	return mp_const_none;
}

static mp_obj_t modsndmixer_resume_all() {
	if (!sndmixer_started) { mp_raise_ValueError(msg_error_not_started); return mp_const_none; }
	sndmixer_resume_all();
	return mp_const_none;
}

static mp_obj_t modsndmixer_loop(mp_obj_t _id, mp_obj_t _loop) {
	if (!sndmixer_started) { mp_raise_ValueError(msg_error_not_started); return mp_const_none; }
	int id = mp_obj_get_int(_id);
	int loop = mp_obj_get_int(_loop);
	sndmixer_set_loop(id, loop);
	return mp_const_none;
}

static mp_obj_t modsndmixer_volume(mp_obj_t _id, mp_obj_t _volume) {
	if (!sndmixer_started) { mp_raise_ValueError(msg_error_not_started); return mp_const_none; }
	int id = mp_obj_get_int(_id);
	int volume = mp_obj_get_int(_volume);
	sndmixer_set_volume(id, volume);
	return mp_const_none;
}

/* Audio sources */

static mp_obj_t modsndmixer_wav(mp_obj_t _data) {
	if (!sndmixer_started) { mp_raise_ValueError(msg_error_not_started); return mp_const_none; }
	mp_uint_t len;
	if (!MP_OBJ_IS_TYPE(_data, &mp_type_bytes)) {
		mp_raise_ValueError("Expected a bytestring like object.");
		return mp_const_none;
	}
	uint8_t *data = (uint8_t *)mp_obj_str_get_data(_data, &len);

	int id=sndmixer_queue_wav(data, data+len-1, 1);
	return mp_obj_new_int(id);
}

static mp_obj_t modsndmixer_mod(mp_obj_t _data) {
	if (!sndmixer_started) { mp_raise_ValueError(msg_error_not_started); return mp_const_none; }
	mp_uint_t len;
	if (!MP_OBJ_IS_TYPE(_data, &mp_type_bytes)) {
		mp_raise_ValueError("Expected a bytestring like object.");
		return mp_const_none;
	}
	uint8_t *data = (uint8_t *)mp_obj_str_get_data(_data, &len);

	int id=sndmixer_queue_mod(data, data+len-1);
	return mp_obj_new_int(id);
}

static mp_obj_t modsndmixer_mp3(mp_obj_t _data) {
	if (!sndmixer_started) { mp_raise_ValueError(msg_error_not_started); return mp_const_none; }
	mp_uint_t len;
	if (!MP_OBJ_IS_TYPE(_data, &mp_type_bytes)) {
		mp_raise_ValueError("Expected a bytestring like object.");
		return mp_const_none;
	}
	uint8_t *data = (uint8_t *)mp_obj_str_get_data(_data, &len);

	int id=sndmixer_queue_mp3(data, data+len-1);
	return mp_obj_new_int(id);
}

static mp_obj_t modsndmixer_synth() {
	if (!sndmixer_started) { mp_raise_ValueError(msg_error_not_started); return mp_const_none; }
	int id=sndmixer_queue_synth();
	return mp_obj_new_int(id);
}

static mp_obj_t modsndmixer_freq(mp_obj_t _id, mp_obj_t _freq) {
	if (!sndmixer_started) { mp_raise_ValueError(msg_error_not_started); return mp_const_none; }
	int id = mp_obj_get_int(_id);
	int freq = mp_obj_get_int(_freq);
	sndmixer_freq(id, freq);
	return mp_const_none;
}

/* --- */
static MP_DEFINE_CONST_FUN_OBJ_1(modsndmixer_begin_obj,      modsndmixer_begin);
static MP_DEFINE_CONST_FUN_OBJ_1(modsndmixer_play_obj,       modsndmixer_play);
static MP_DEFINE_CONST_FUN_OBJ_1(modsndmixer_pause_obj,      modsndmixer_pause);
static MP_DEFINE_CONST_FUN_OBJ_1(modsndmixer_stop_obj,       modsndmixer_stop);
static MP_DEFINE_CONST_FUN_OBJ_0(modsndmixer_pause_all_obj,  modsndmixer_pause_all);
static MP_DEFINE_CONST_FUN_OBJ_0(modsndmixer_resume_all_obj, modsndmixer_resume_all);
static MP_DEFINE_CONST_FUN_OBJ_2(modsndmixer_loop_obj,       modsndmixer_loop);
static MP_DEFINE_CONST_FUN_OBJ_2(modsndmixer_volume_obj,     modsndmixer_volume);
static MP_DEFINE_CONST_FUN_OBJ_1(modsndmixer_wav_obj,        modsndmixer_wav);
static MP_DEFINE_CONST_FUN_OBJ_1(modsndmixer_mod_obj,        modsndmixer_mod);
static MP_DEFINE_CONST_FUN_OBJ_1(modsndmixer_mp3_obj,        modsndmixer_mp3);
static MP_DEFINE_CONST_FUN_OBJ_0(modsndmixer_synth_obj,      modsndmixer_synth);
static MP_DEFINE_CONST_FUN_OBJ_2(modsndmixer_freq_obj,       modsndmixer_freq);

static const mp_rom_map_elem_t sndmixer_module_globals_table[] = {
	{MP_ROM_QSTR(MP_QSTR_begin),      MP_ROM_PTR(&modsndmixer_begin_obj)},
	{MP_ROM_QSTR(MP_QSTR_play),       MP_ROM_PTR(&modsndmixer_play_obj)},
	{MP_ROM_QSTR(MP_QSTR_pause),      MP_ROM_PTR(&modsndmixer_pause_obj)},
	{MP_ROM_QSTR(MP_QSTR_stop),       MP_ROM_PTR(&modsndmixer_stop_obj)},
	{MP_ROM_QSTR(MP_QSTR_pause_all),  MP_ROM_PTR(&modsndmixer_pause_all_obj)},
	{MP_ROM_QSTR(MP_QSTR_resume_all), MP_ROM_PTR(&modsndmixer_resume_all_obj)},
	{MP_ROM_QSTR(MP_QSTR_loop),       MP_ROM_PTR(&modsndmixer_loop_obj)},
	{MP_ROM_QSTR(MP_QSTR_volume),     MP_ROM_PTR(&modsndmixer_volume_obj)},
	{MP_ROM_QSTR(MP_QSTR_wav),        MP_ROM_PTR(&modsndmixer_wav_obj)},
	{MP_ROM_QSTR(MP_QSTR_mod),        MP_ROM_PTR(&modsndmixer_mod_obj)},
	{MP_ROM_QSTR(MP_QSTR_mp3),        MP_ROM_PTR(&modsndmixer_mp3_obj)},
	{MP_ROM_QSTR(MP_QSTR_synth),      MP_ROM_PTR(&modsndmixer_synth_obj)},
	{MP_ROM_QSTR(MP_QSTR_freq),       MP_ROM_PTR(&modsndmixer_freq_obj)},
};

static MP_DEFINE_CONST_DICT(sndmixer_module_globals, sndmixer_module_globals_table);

const mp_obj_module_t sndmixer_module = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *)&sndmixer_module_globals,
};

#endif
