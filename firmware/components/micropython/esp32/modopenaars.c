#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include <driver_openaars.h>

#ifdef CONFIG_DRIVER_OPENAARS_ENABLE

// Initialize the SPI interface
static mp_obj_t openaars_init_(void){

  esp_err_t res = driver_openaars_init();
	if (res != ESP_OK) {
		mp_raise_OSError(MP_EIO);
	}

	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(openaars_init_obj, openaars_init_);

// send into the SPI interface
static mp_obj_t openaars_send_(mp_obj_t _data){
  // Retrieve the arguments
	mp_uint_t data_len;
	uint8_t *data = (uint8_t *) mp_obj_str_get_data(_data, &data_len);

  // Send the command to the backend driver
  esp_err_t res = driver_openaars_send(data, data_len, false );
	if (res != ESP_OK) {
		mp_raise_OSError(MP_EIO);
	}

  // Nothing to return :-(
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(openaars_send_obj, openaars_send_);

// Receive from the SPI interface
static mp_obj_t openaars_receive_(mp_obj_t len_in){
  // Decode arguments
  size_t len = mp_obj_get_int(len_in);

  // Create the buffer to receive data in
  vstr_t vstr;
  vstr_init_len(&vstr, len);

  // Receive the data from the SPI backend driver
  esp_err_t ret = driver_openaars_receive((uint8_t *)vstr.buf, len, false);
  if (ret != ESP_OK) {
		mp_raise_OSError(MP_EIO);
  }

  // Create the new return bytes object and return it to Python
  vstr.len = ret;
  return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
static MP_DEFINE_CONST_FUN_OBJ_1(openaars_receive_obj, openaars_receive_);

// Register the interrupt handlers for the FPGA and ADV interrupts
static mp_obj_t interrupt_callbacks[2] = {
	mp_const_none, mp_const_none
};

// Interrupt handler to call the Python handler routine
static void openaars_event_handler(void *b, bool state)
{
	int pin = (uint32_t) b;
	if ((pin < 0) || (pin > 1)) return;
	if(interrupt_callbacks[pin] != mp_const_none){
		if ((!MP_OBJ_IS_FUN(interrupt_callbacks[pin])) && (!MP_OBJ_IS_METH(interrupt_callbacks[pin]))) {
			printf("MPR121 ERROR: CALLBACK IS NOT FUNCTION OR METHOD?!?! (pin %u)\n", pin);
		} else {
			mp_sched_schedule(interrupt_callbacks[pin], mp_obj_new_bool(state), NULL);
		}
	}
}

// Attach interupt handler
static mp_obj_t openaars_interrupt_attach_(mp_obj_t _pin, mp_obj_t _func) {
	int pin = mp_obj_get_int(_pin);
	if ((pin < 0) || (pin > 1)) return mp_const_none; // 0 = FPGA, 1 = ADV7511
	driver_openaars_set_interrupt_handler(pin, openaars_event_handler, (void*) (pin));
	if ((!MP_OBJ_IS_FUN(_func) && (!MP_OBJ_IS_METH(_func)))) {
		mp_raise_ValueError("callback function expected");
		return mp_const_none;
	}
	interrupt_callbacks[pin] = _func;
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(openaars_interrupt_attach_obj, openaars_interrupt_attach_);

// Detach interrupt handler
static mp_obj_t openaars_interrupt_detach_(mp_obj_t _pin) {
  int pin = mp_obj_get_int(_pin);
  if ((pin < 0) || (pin > 1)) return mp_const_none;
  interrupt_callbacks[pin] = mp_const_none;
  return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(openaars_interrupt_detach_obj, openaars_interrupt_detach_);

// Create globals table
static const mp_rom_map_elem_t openaars_module_globals_table[] = {
  // Constants
	{MP_ROM_QSTR( MP_QSTR_OPENAARS_ADV_INT), MP_ROM_INT(0)},  // FPGA interrupt number
	{MP_ROM_QSTR( MP_QSTR_OPENAARS_FPGA_INT), MP_ROM_INT(1)}, // ADV interrupt number
  // Methods
	{MP_OBJ_NEW_QSTR(MP_QSTR_init), MP_ROM_PTR(&openaars_init_obj)},
	{MP_OBJ_NEW_QSTR(MP_QSTR_send), MP_ROM_PTR(&openaars_send_obj)},
	{MP_OBJ_NEW_QSTR(MP_QSTR_receive), MP_ROM_PTR(&openaars_receive_obj)},
	{MP_OBJ_NEW_QSTR(MP_QSTR_attach), MP_ROM_PTR(&openaars_interrupt_attach_obj)},
	{MP_OBJ_NEW_QSTR(MP_QSTR_detach), MP_ROM_PTR(&openaars_interrupt_detach_obj)},
};

// Create dictionary
static MP_DEFINE_CONST_DICT(openaars_module_globals, openaars_module_globals_table);

// Create the module object
const mp_obj_module_t openaars_module = {
	.base = {&mp_type_module},
	.globals = (mp_obj_dict_t *)&openaars_module_globals,
};

  
 
#endif // CONFIG_DRIVER_OPENAARS_ENABLE
