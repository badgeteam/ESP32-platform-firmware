#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

#define RX_BUFFER_SIZE 32

typedef struct DynamicBuffer_t {
	struct DynamicBuffer_t* next;
	uint8_t* buffer;
	uint32_t position;
	uint32_t length;
} DynamicBuffer;

DynamicBuffer* in_queue  = NULL;
DynamicBuffer* out_queue = NULL;

static mp_obj_t callback = NULL;

/* Used in mphalport */

bool loopback_rx_any() {
	return (in_queue != NULL);
}

int loopback_rx_char() {
	if (in_queue == NULL) return 0; //EOF
	int result = 0;
	DynamicBuffer* queue = in_queue;
	while (queue) {
		if (queue->position < queue->length) {
			//Data available in current buffer!
			result = queue->buffer[queue->position];
			queue->position++;
		}
		if (queue->position >= queue->length) {
			//No more data in current buffer, free and move to next
			free(queue->buffer);
			in_queue = queue->next;
			free(queue);
			queue = in_queue;
		}
		if (result > 0) {
			break;
		}
	}
	return result;
}

void loopback_stdout_put(const char* str, uint32_t len) {
	if (callback) {
		mp_sched_schedule(callback, mp_obj_new_str(str, len), NULL);
	}
}

/* Internal functions */

bool buffer_put(DynamicBuffer** buffer, const char* str, uint32_t len) {
	DynamicBuffer* currentQueue = *buffer;
	DynamicBuffer* nextQueue    = *buffer;
	while (nextQueue) {
		currentQueue = nextQueue;
		nextQueue = currentQueue->next;
	}
	//currentQueue is now the last item in the queue!
	nextQueue = calloc(1, sizeof(DynamicBuffer));
	if (!nextQueue) return false; //No memory
	nextQueue->buffer = malloc(len);
	if (!nextQueue->buffer) { //No memory
		free(nextQueue);
		return false;
	}
	memcpy(nextQueue->buffer, str, len);
	nextQueue->length = len;
	if (currentQueue) {
		currentQueue->next = nextQueue;
	} else {
		*buffer = nextQueue;
	}
	return true;
}

bool stdin_put(char* str, uint32_t len) {
	return buffer_put(&in_queue, str, len);
}

/* Python API */

static mp_obj_t loopback_attach(mp_obj_t _func) {
	if ((!MP_OBJ_IS_FUN(_func) && (!MP_OBJ_IS_METH(_func)))) {
		mp_raise_ValueError("Expected argument to be a function");
		return mp_const_none;
	}
	callback = _func;
	return mp_const_none;
}

static mp_obj_t loopback_detach() {
	callback = NULL;
	return mp_const_none;
}

static mp_obj_t loopback_put(mp_uint_t n_args, const mp_obj_t *args) {
	if (! MP_OBJ_IS_TYPE(args[0], &mp_type_bytes)) {
		mp_raise_ValueError("Expected a bytestring like object.");
		return mp_const_none;
	}
	mp_uint_t len;
	char *data = (char *)mp_obj_str_get_data(args[0], &len);	
	return mp_obj_new_bool(stdin_put(data, len));
}

static MP_DEFINE_CONST_FUN_OBJ_1(loopback_attach_obj, loopback_attach );
static MP_DEFINE_CONST_FUN_OBJ_0(loopback_detach_obj, loopback_detach );
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(loopback_put_obj, 1, 1, loopback_put );

static const mp_rom_map_elem_t loopback_module_globals_table[] = {
	{MP_OBJ_NEW_QSTR(MP_QSTR_attach), (mp_obj_t)&loopback_attach_obj},
	{MP_OBJ_NEW_QSTR(MP_QSTR_detach), (mp_obj_t)&loopback_detach_obj},
	{MP_OBJ_NEW_QSTR(MP_QSTR_put), (mp_obj_t)&loopback_put_obj},
};

static MP_DEFINE_CONST_DICT(loopback_module_globals, loopback_module_globals_table);

const mp_obj_module_t loopback_module = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *)&loopback_module_globals,
};
