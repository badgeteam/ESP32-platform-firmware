# CampZone2019 badge specific input wrapper
# Versions for other badges must expose the same API

import _buttons, system

# --- BUTTON CONSTANTS  ---
BTN_UP     = 0
BTN_DOWN   = 1
BTN_LEFT   = 2
BTN_RIGHT  = 3
BTN_A      = 4
BTN_B      = 5

# --- INTERNAL MAPPING TABLES ---

__num = 6
_gpioMap = [26, 27, 32, 33, 36, 39]

# --- CALLBACKS ---
__cb = []

# --- DEFAULT ACTION ---
def __cbReboot(pressed):
	if pressed:
		system.reboot()

# --- INTERNAL CALLBACK WRAPPERS ---

def __cb_btn_a(arg):
	if __cb[-1][BTN_A]:
		__cb[-1][BTN_A](arg)

def __cb_btn_b(arg):
	if __cb[-1][BTN_B]:
		__cb[-1][BTN_B](arg)

def __cb_btn_up(arg):
	if __cb[-1][BTN_UP]:
		__cb[-1][BTN_UP](arg)

def __cb_btn_down(arg):
	if __cb[-1][BTN_DOWN]:
		__cb[-1][BTN_DOWN](arg)

def __cb_btn_left(arg):
	if __cb[-1][BTN_LEFT]:
		__cb[-1][BTN_LEFT](arg)

def __cb_btn_right(arg):
	if __cb[-1][BTN_RIGHT]:
		__cb[-1][BTN_RIGHT](arg)

def __init():
	global mappings
	_buttons.register( _gpioMap[BTN_A],     __cb_btn_a     )
	_buttons.register( _gpioMap[BTN_B],     __cb_btn_b     )
	_buttons.register( _gpioMap[BTN_UP],    __cb_btn_up    )
	_buttons.register( _gpioMap[BTN_DOWN],  __cb_btn_down  )
	_buttons.register( _gpioMap[BTN_LEFT],  __cb_btn_left  )
	_buttons.register( _gpioMap[BTN_RIGHT], __cb_btn_right )
	pushMapping() #Add the initial / default mapping
	
# --- PUBLIC API ---

def attach(button, callback):
	# This function attachs a callback to a button
	global __num, __cb
	if button < 0 or button >= __num:
		raise ValueError("Invalid button!")
	__cb[-1][button] = callback

def detach(button):
	# This function removes the callback of a button
	global __num, __cb
	if button < 0 or button >= __num:
		raise ValueError("Invalid button!")
	__cb[-1][button] = None

def value(button):
	# Reads the state of a button
	global _buttons, __mprMap, __num
	if button < 0 or button >= __num:
		raise ValueError("Invalid button!")
	return not _buttons.pin(_gpioMap[button]).value()

def getCallback(button):
	# Returns the currently attached callback function
	global __num, __cb
	if button < 0 or button >= __num:
		raise ValueError("Invalid button!")
	return __cb[-1][button]

def pushMapping(newMapping=None):
	global __cb
	if newMapping == None:
		newMapping = { BTN_UP: None, BTN_DOWN: None, BTN_LEFT: None, BTN_RIGHT: None, BTN_A: None, BTN_B: __cbReboot }
	__cb.append(newMapping)

def popMapping():
	global __cb
	if len(__cb) > 0:
		__cb = __cb[:-1]
	if len(__cb) < 1:
		pushMapping()
	

# --- CampZone2019 specific legacy API compatibility ---

def init_button_mapping():
	pushMapping()
	return True

def clear_button_mapping():
	popMapping()
	return True

def assign(button, action = None):
	attach(button, action)
	return True

register = assign

def unassign(button):
	return assign(button, None)

# ---
__init()
