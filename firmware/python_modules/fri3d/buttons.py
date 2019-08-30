# Fri3d2018 badge specific input wrapper
# Versions for other badges must expose the same API

import _buttons, system

# --- BUTTON CONSTANTS  ---
BTN_0      = 0
BTN_1      = 1
BTN_BOOT   = 2

# --- INTERNAL MAPPING TABLES ---

__num = 3
_gpioMap = [36, 39, 0]

# --- CALLBACKS ---
__cb = []

# --- DEFAULT ACTION ---
def __cbReboot(pressed):
	if pressed:
		system.reboot()

# --- INTERNAL CALLBACK WRAPPERS ---

def __cb_btn_0(arg):
	if __cb[-1][BTN_0]:
		__cb[-1][BTN_0](arg)

def __cb_btn_1(arg):
	if __cb[-1][BTN_1]:
		__cb[-1][BTN_1](arg)

def __cb_btn_boot(arg):
	if __cb[-1][BTN_BOOT]:
		__cb[-1][BTN_BOOT](arg)

def __init():
	global mappings
	_buttons.register( _gpioMap[BTN_0],     __cb_btn_0     )
	_buttons.register( _gpioMap[BTN_1],     __cb_btn_1     )
	_buttons.register( _gpioMap[BTN_BOOT],  __cb_btn_boot  )
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
		newMapping = { BTN_0: None, BTN_1: None, BTN_BOOT: None }
	__cb.append(newMapping)

def popMapping():
	global __cb
	if len(__cb) > 0:
		__cb = __cb[:-1]
	if len(__cb) < 1:
		pushMapping()

# Extremely basic touchpad support for fri3d 2018
__touch1 = machine.TouchPad(machine.Pin(12))
__touch2 = machine.TouchPad(machine.Pin(14))

def touch1():
	return __touch1.read()

def touch2():
	return __touch2.read()

# ---
__init()
