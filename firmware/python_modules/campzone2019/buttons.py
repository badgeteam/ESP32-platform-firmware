import machine, system, time
from defines import *

_gpios     = []
_pins      = []

button_mappings = []

def init_button_mapping():
	global button_mappings
	_register_all_handlers()
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
	print('_cb', pin)
	position = _pins.index(pin)
	gpio = _gpios[position]
	callback = button_mappings[-1][gpio]
	if callback and callable(callback):
		callback(not pin.value())
	else:
		print('No callback for button')
	
def _register(gpio):
	pin = machine.Pin(gpio, machine.Pin.IN, handler=_cb, trigger=machine.Pin.IRQ_ANYEDGE, debounce=200, acttime=200)
	if gpio in _gpios:
		index = _gpios.index(gpio)
		_pins[index] = pin
		return False
	_gpios.append(gpio)
	_pins.append(pin)
	return True

def _default_B_action(pressed):
	if pressed:
		# Make apps reboot when B is pressed, unless they rebind it
		print('Exit by B button')
		system.reboot()

def _default_button_mapping():
	return {
		BTN_UP: None,
		BTN_DOWN: None,
		BTN_LEFT: None,
		BTN_RIGHT: None,
		BTN_A: None,
		BTN_B: _default_B_action
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
