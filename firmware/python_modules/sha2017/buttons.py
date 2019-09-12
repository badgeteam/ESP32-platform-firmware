# SHA2017 badge specific input wrapper
# Versions for other badges must expose the same API

import mpr121, _buttons, _mpr121mapping, system, machine

# --- BUTTON CONSTANTS  ---
BTN_A      = 0
BTN_B      = 1
BTN_START  = 2
BTN_SELECT = 3
BTN_DOWN   = 4
BTN_RIGHT  = 5
BTN_UP     = 6
BTN_LEFT   = 7
BTN_FLASH  = 8

# --- INTERNAL MAPPING TABLES ---

__num = 9
__mprMap = _mpr121mapping.buttons

# --- INTERNAL VARIABLES ---
__orientation = 0

# --- CALLBACKS ---
__cb = []

# --- DEFAULT ACTION ---
def __cbReboot(pressed):
	if pressed:
		system.launcher()

# --- INTERNAL CALLBACK WRAPPERS ---

def __cb_btn_a(arg):
	if __cb[-1][BTN_A]:
		__cb[-1][BTN_A](arg)

def __cb_btn_b(arg):
	if __cb[-1][BTN_B]:
		__cb[-1][BTN_B](arg)

def __cb_btn_start(arg):
	if __cb[-1][BTN_START]:
		__cb[-1][BTN_START](arg)

def __cb_btn_select(arg):
	if __cb[-1][BTN_SELECT]:
		__cb[-1][BTN_SELECT](arg)

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

def __cb_btn_flash(arg):
	if __cb[-1][BTN_FLASH]:
		__cb[-1][BTN_FLASH](arg)

def __init():
	global mappings
	_buttons.register( 0,                    __cb_btn_flash  ) # The flash button is connected to the badge on GPIO 0
	mpr121.attach    ( __mprMap[BTN_A],      __cb_btn_a      ) # The A button is connected to input 0 of the MPR121
	mpr121.attach    ( __mprMap[BTN_B],      __cb_btn_b      ) # The B button is connected to input 1 of the MPR121
	mpr121.attach    ( __mprMap[BTN_START],  __cb_btn_start  ) # The START button is connected to input 2 of the MPR121
	mpr121.attach    ( __mprMap[BTN_SELECT], __cb_btn_select ) # The SELECT button is connected to input 3 of the MPR121
	mpr121.attach    ( __mprMap[BTN_DOWN],   __cb_btn_down   ) # The DOWN button is connected to input 4 of the MPR121
	mpr121.attach    ( __mprMap[BTN_RIGHT],  __cb_btn_right  ) # The RIGHT button is connected to input 5 of the MPR121
	mpr121.attach    ( __mprMap[BTN_UP],     __cb_btn_up     ) # The UP button is connected to input 6 of the MPR121
	mpr121.attach    ( __mprMap[BTN_LEFT],   __cb_btn_left   ) # The LEFT button is connected to input 7 of the MPR121
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
	if button == BTN_FLASH:
		return not _buttons.pin(0).value() # This input has is active LOW
	else:
		return mpr121.get(__mprMap[button])

def getCallback(button):
	# Returns the currently attached callback function
	global __num, __cb
	if button < 0 or button >= __num:
		raise ValueError("Invalid button!")
	return __cb[-1][button]

def pushMapping(newMapping=None):
	global __cb
	if newMapping == None:
		newMapping = { BTN_UP: None, BTN_DOWN: None, BTN_LEFT: None, BTN_RIGHT: None, BTN_A: None, BTN_B: None, BTN_SELECT: None, BTN_START: None }
		if machine.nvs_getint("system", 'factory_checked'):
			newMapping[BTN_START] = __cbReboot
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
