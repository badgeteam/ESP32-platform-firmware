import machine
from defines import *

_gpios     = []
_pins      = []

button_mappings = []

def init_button_mapping():
	global button_mappings
	button_mappings.append(_default_button_mapping())
	return True

def clear_button_mapping():
	global button_mappings
	if len(button_mappings) > 1:
		button_mappings = button_mappings[:-1]
	return True

def assign(gpio, action = None):
	global button_mappings
	button_mappings[-1][gpio] = action
	return True
	
def unassign(gpio):
	return assign(gpio, None)

def _cb(pin):
	position = _pins.index(pin)
	gpio = _gpios[position]
	callback = button_mappings[-1][gpio]
	if callback and callable(callback):
		callback(not pin.value())
	
def _register(gpio):
	if gpio in _gpios:
		return False
	pin = machine.Pin(gpio, machine.Pin.IN, handler=_cb, trigger=machine.Pin.IRQ_ANYEDGE, debounce=100, acttime=100)
	_gpios.append(gpio)
	_pins.append(pin)
	return True

def _default_button_mapping():
	return {
		BTN_UP: None,
		BTN_DOWN: None,
		BTN_LEFT: None,
		BTN_RIGHT: None,
		BTN_A: None,
		BTN_B: None
	}

def _register_all_handlers():
	_register(BTN_UP)
	_register(BTN_DOWN)
	_register(BTN_LEFT)
	_register(BTN_RIGHT)
	_register(BTN_A)
	_register(BTN_B)

register = assign
init_button_mapping()
_register_all_handlers()
