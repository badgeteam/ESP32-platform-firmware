# DISOBEY2019 badge specific input wrapper
# Versions for other badges must expose the same API

import _buttons, system, machine, samd

# --- BUTTON CONSTANTS  ---
BTN_LEFT   = 0
BTN_UP     = 1
BTN_BACK   = 2
BTN_OK     = 3
BTN_DOWN   = 4
BTN_RIGHT  = 5

BTN_A      = 3 # Map A to OK
BTN_B      = 2 # Map B to Back

# --- INTERNAL MAPPING TABLES ---

__num = 6

# --- INTERNAL VARIABLES ---
__orientation = 0

# --- CALLBACKS ---
__cb = []

# --- DEFAULT ACTION ---
def __cbReboot(pressed):
	if pressed:
		system.launcher()

# --- INTERNAL CALLBACK WRAPPERS ---

def __cb_btn_ok(arg):
	if __cb[-1][BTN_OK]:
		__cb[-1][BTN_OK](arg)

def __cb_btn_back(arg):
	if __cb[-1][BTN_BACK]:
		__cb[-1][BTN_BACK](arg)

def __cb_btn_down(arg):
	btn = BTN_DOWN
	if __orientation == 90:
		btn = BTN_LEFT
	elif __orientation == 180:
		btn = BTN_UP
	elif __orientation == 270:
		btn = BTN_RIGHT
	if __cb[-1][btn]:
		__cb[-1][btn](arg)

def __cb_btn_right(arg):
	btn = BTN_RIGHT
	if __orientation == 90:
		btn = BTN_DOWN
	elif __orientation == 180:
		btn = BTN_LEFT
	elif __orientation == 270:
		btn = BTN_UP
	if __cb[-1][btn]:
		__cb[-1][btn](arg)

def __cb_btn_up(arg):
	btn = BTN_UP
	if __orientation == 90:
		btn = BTN_RIGHT
	elif __orientation == 180:
		btn = BTN_DOWN
	elif __orientation == 270:
		btn = BTN_LEFT
	if __cb[-1][btn]:
		__cb[-1][btn](arg)

def __cb_btn_left(arg):
	btn = BTN_LEFT
	if __orientation == 90:
		btn = BTN_UP
	elif __orientation == 180:
		btn = BTN_RIGHT
	elif __orientation == 270:
		btn = BTN_DOWN
	if __cb[-1][btn]:
		__cb[-1][btn](arg)

def __init():
	global mappings
	samd.attach( BTN_LEFT,   __cb_btn_left   )
	samd.attach( BTN_UP,     __cb_btn_up     )
	samd.attach( BTN_BACK,   __cb_btn_back   )
	samd.attach( BTN_OK,     __cb_btn_ok     )
	samd.attach( BTN_DOWN,   __cb_btn_down   )
	samd.attach( BTN_RIGHT,  __cb_btn_right  )
	
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
	global _buttons, __samdMap, __num
	if button < 0 or button >= __num:
		raise ValueError("Invalid button!")
	return (samd.read_touch() >> button) & 0x01

def getCallback(button):
	# Returns the currently attached callback function
	global __num, __cb
	if button < 0 or button >= __num:
		raise ValueError("Invalid button!")
	return __cb[-1][button]

def pushMapping(newMapping=None):
	global __cb
	if newMapping == None:
		newMapping = { BTN_UP: None, BTN_DOWN: None, BTN_LEFT: None, BTN_RIGHT: None, BTN_OK: None, BTN_BACK: None }
		if machine.nvs_getint("system", 'factory_checked'):
			newMapping[BTN_BACK] = __cbReboot
	__cb.append(newMapping)

def popMapping():
	global __cb
	if len(__cb) > 0:
		__cb = __cb[:-1]
	if len(__cb) < 1:
		pushMapping()

def rotate(value):
	global __orientation
	__orientation = value

# ---
__init()
