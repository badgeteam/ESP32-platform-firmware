# TTGO T-Display badge specific input wrapper
# Versions for other badges must expose the same API

import _buttons, system

# --- BUTTON CONSTANTS  ---
BTN_A      = 0
BTN_B      = 1

# --- INTERNAL MAPPING TABLES ---

__num = 2
_gpioMap = [35, 0]

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

def __init():
	global mappings
	_buttons.register( _gpioMap[BTN_A],     __cb_btn_a     )
	_buttons.register( _gpioMap[BTN_B],     __cb_btn_b     )
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
		newMapping = { BTN_A: None, BTN_B: __cbReboot }
	__cb.append(newMapping)

def popMapping():
	global __cb
	if len(__cb) > 0:
		__cb = __cb[:-1]
	if len(__cb) < 1:
		pushMapping()

# ---
__init()
