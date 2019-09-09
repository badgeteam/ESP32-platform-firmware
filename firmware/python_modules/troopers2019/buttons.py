# Troopers 2019 badge specific input wrapper
# Versions for other badges must expose the same API

import _buttons, system, machine

from pca9539a import PCA9539A
from pca9555 import PCA9555
from machine import Pin, I2C

# --- BUTTON CONSTANTS  ---
BTN_A         = 0
BTN_B         = 1
BTN_START     = 2
BTN_SELECT    = 3
BTN_DOWN      = 4
BTN_RIGHT     = 5
BTN_UP        = 6
BTN_LEFT      = 7
BTN_FLASH     = 8

KEY_Q         = 9
KEY_W         = 10
KEY_E         = 11
KEY_R         = 12
KEY_T         = 13
KEY_Y         = 14
KEY_U         = 15
KEY_I         = 16
KEY_O         = 17
KEY_P         = 18
KEY_A         = 19
KEY_S         = 20
KEY_D         = 21
KEY_F         = 22
KEY_G         = 23
KEY_H         = 24
KEY_J         = 25
KEY_K         = 26
KEY_L         = 27
KEY_Z         = 28
KEY_X         = 29
KEY_C         = 30
KEY_V         = 31
KEY_B         = 32
KEY_N         = 33
KEY_M         = 34
KEY_RETURN    = 35
KEY_SHIELD    = 36
KEY_FN        = 37
KEY_SPACE     = 38
KEY_BACKSPACE = 39
KEY_SHIFT     = 40

KEY_ANY       = 41

# --- INTERNAL MAPPING TABLES ---

__num = 42

# --- INTERNAL VARIABLES ---
__orientation = 0

# --- CALLBACKS ---
__cb = []

# --- DEFAULT ACTION ---
def __cbReboot(pressed):
	if pressed:
		system.launcher()

# --- INTERNAL CALLBACK WRAPPERS ---

def __apply_orientation(btn):
	if btn == BTN_UP:
		if __orientation == 90:
			btn = BTN_RIGHT
		elif __orientation == 180:
			btn = BTN_DOWN
		elif __orientation == 270:
			btn = BTN_LEFT
	elif btn == BTN_DOWN:
		if __orientation == 90:
			btn = BTN_LEFT
		elif __orientation == 180:
			btn = BTN_UP
		elif __orientation == 270:
			btn = BTN_RIGHT
	elif btn == BTN_LEFT:
		if __orientation == 90:
			btn = BTN_UP
		elif __orientation == 180:
			btn = BTN_RIGHT
		elif __orientation == 270:
			btn = BTN_DOWN
	elif btn == BTN_RIGHT:
		if __orientation == 90:
			btn = BTN_DOWN
		elif __orientation == 180:
			btn = BTN_LEFT
		elif __orientation == 270:
			btn = BTN_UP
	return btn

def __cb_nav(btn, state):
	if btn < 0:
		print("[BUTTONS] received invalid input (NAV)!")
		return
	btn = __apply_orientation(btn)
	if __cb[-1][btn]:
		__cb[-1][btn](state)

def __cb_kb(btn, state):
	if btn < 0:
		print("[BUTTONS] received invalid input (KB)!")
		return
	if __cb[-1][btn]:
		__cb[-1][btn](state)
	elif __cb[-1][KEY_ANY]:
		__cb[-1][KEY_ANY](btn, state)

def __cb_btn_flash(arg):
	if __cb[-1][BTN_FLASH]:
		__cb[-1][BTN_FLASH](arg)
	
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
	global _buttons, __num
	if button < 0 or button >= __num:
		raise ValueError("Invalid button!")
	if button == BTN_FLASH:
		return not _buttons.pin(0).value() # This input has is active LOW
	else:
		return 0

def getCallback(button):
	# Returns the currently attached callback function
	global __num, __cb
	if button < 0 or button >= __num:
		raise ValueError("Invalid button!")
	return __cb[-1][button]

def pushMapping(newMapping=None):
	global __cb
	if newMapping == None:
		newMapping = { BTN_UP: None, BTN_DOWN: None, BTN_LEFT: None, BTN_RIGHT: None, BTN_A: None, BTN_B: None, BTN_SELECT: None, BTN_START: None, KEY_G: None, KEY_B: None, KEY_H: None, KEY_RETURN: None, KEY_M: None, KEY_N: None, KEY_SHIFT: None, KEY_BACKSPACE: None, KEY_J: None, KEY_K: None, KEY_L: None, KEY_Y: None, KEY_U: None, KEY_I: None, KEY_O: None, KEY_P: None, KEY_Q: None, KEY_A: None, KEY_Z: None, KEY_SHIELD: None, KEY_W: None, KEY_S: None, KEY_X: None, KEY_FN: None, KEY_T: None, KEY_V: None, KEY_F: None, KEY_R: None, KEY_SPACE: None, KEY_C: None, KEY_D: None, KEY_E: None, KEY_ANY: None }
		if machine.nvs_getint("system", 'factory_checked'): # Do NOT assign this key when running the factory check!
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

__i2c = I2C(scl=Pin(5), sda=Pin(4), freq=400000)
__ioConsole = PCA9539A(__i2c, 0x77, [-1, -1, -1, -1, -1, -1, -1, -1, BTN_START, BTN_B, BTN_A, BTN_SELECT, BTN_UP, BTN_RIGHT, BTN_LEFT, BTN_DOWN], Pin(39), __cb_nav, wakeup=True, inverted=True)
__ioConsole.init()

__ioKeyboard0 = PCA9555(__i2c, 0x25, [KEY_G, KEY_B, KEY_H, KEY_RETURN, KEY_M, KEY_N, KEY_SHIFT, KEY_BACKSPACE, KEY_J, KEY_K, KEY_L, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P], Pin(35), __cb_kb, inverted=True)
__ioKeyboard0.init()

__ioKeyboard1 = PCA9555(__i2c, 0x24, [KEY_Q, KEY_A, KEY_Z, KEY_SHIELD, KEY_W, KEY_S, KEY_X, KEY_FN, KEY_T, KEY_V, KEY_F, KEY_R, KEY_SPACE, KEY_C, KEY_D, KEY_E], Pin(34), __cb_kb, inverted=True)
__ioKeyboard1.init()

_buttons.register( 0, __cb_btn_flash ) # The flash button is connected to the badge on GPIO 0
pushMapping() #Add the initial / default mapping
