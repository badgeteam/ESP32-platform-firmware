import machine

_gpios     = []
_pins      = []
_callbacks = []

def _cb(pin):
	position = _pins.index(pin)
	gpio = _gpios[position]
	callback = _callbacks[position]
	if callable(callback):
		callback(not pin.value())
	
def register(gpio, action=None):
	if gpio in _gpios:
		return False
	pin = machine.Pin(gpio, machine.Pin.IN, handler=_cb, trigger=machine.Pin.IRQ_ANYEDGE, debounce=100, acttime=100)
	_gpios.append(gpio)
	_pins.append(pin)
	_callbacks.append(action)
	return True

def assign(gpio, action):
	if not gpio in _gpios:
		return False
	position = _gpios.index(gpio)
	_callbacks[position] = action
	return True
	
def unassign(gpio):
	return assign(gpio, None)
