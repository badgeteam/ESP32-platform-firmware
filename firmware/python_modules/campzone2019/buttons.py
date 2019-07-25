import machine, system
from defines import *

def _cb(pin):
	position = _pins.index(pin)
	gpio = _gpios[position]
	callback = button_mappings[-1][gpio]
	if callback and callable(callback):
		callback(not pin.value())

_gpios     = [BTN_A, BTN_B, BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT]
_pins      = [
	machine.Pin(BTN_A, machine.Pin.IN, handler=_cb, trigger=machine.Pin.IRQ_ANYEDGE, debounce=100, acttime=100),
	machine.Pin(BTN_B, machine.Pin.IN, handler=_cb, trigger=machine.Pin.IRQ_ANYEDGE, debounce=100, acttime=100),
	machine.Pin(BTN_UP, machine.Pin.IN, handler=_cb, trigger=machine.Pin.IRQ_ANYEDGE, debounce=100, acttime=100),
	machine.Pin(BTN_DOWN, machine.Pin.IN, handler=_cb, trigger=machine.Pin.IRQ_ANYEDGE, debounce=100, acttime=100),
	machine.Pin(BTN_LEFT, machine.Pin.IN, handler=_cb, trigger=machine.Pin.IRQ_ANYEDGE, debounce=100, acttime=100),
	machine.Pin(BTN_RIGHT, machine.Pin.IN, handler=_cb, trigger=machine.Pin.IRQ_ANYEDGE, debounce=100, acttime=100),
]

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

def _register(gpio):
	if gpio in _gpios:
		return False
	pin = machine.Pin(gpio, machine.Pin.IN, handler=_cb, trigger=machine.Pin.IRQ_ANYEDGE, debounce=100, acttime=100)
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

register = assign
init_button_mapping()
