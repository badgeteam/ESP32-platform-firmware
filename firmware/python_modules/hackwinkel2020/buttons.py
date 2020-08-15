# Hackwinkel 2020 badge specific input wrapper
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
__orientation = 0
_gpioMap = [34,36,35,39,25,26]

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

def rotate(value):
    '''
    Change the orientation of the arrow keys (0, 90, 180, 270)
    '''
    global __orientation
    __orientation = value

__init()
