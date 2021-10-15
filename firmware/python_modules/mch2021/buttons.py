# MCH2021 button input wrapper
# Versions of this file tailored for other badges should expose the same API

import pca9555, _buttons, _pca9555mapping, system, machine

# --- BUTTON CONSTANTS  ---
BTN_A        = 0
BTN_B        = 1
BTN_START    = 2
BTN_SELECT   = 3
BTN_DOWN     = 4
BTN_RIGHT    = 5
BTN_UP       = 6
BTN_LEFT     = 7
BTN_HOME     = 8
BTN_MENU     = 9
BTN_JOY      = 10
PIN_CHARGING = 11

# Internal variables
__num = 12
__pcaMap = _pca9555mapping.buttons
__orientation = 0
__cb = []

# Internal functions
def __cbReboot(pressed):
    if pressed:
        system.launcher()

def __cbHome(pressed):
    if pressed:
        system.home()

def __cbMenu(pressed):
    if pressed:
        system.launcher()

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
        
def __cb_btn_home(arg):
    if __cb[-1][BTN_HOME]:
        __cb[-1][BTN_HOME](arg)

def __cb_btn_menu(arg):
    if __cb[-1][BTN_MENU]:
        __cb[-1][BTN_MENU](arg)

def __cb_btn_joy(arg):
    if __cb[-1][BTN_JOY]:
        __cb[-1][BTN_JOY](arg)

def __cb_pin_charging(arg):
    if __cb[-1][PIN_CHARGING]:
        __cb[-1][PIN_CHARGING](arg)

def __init():
    pca9555.attach    ( __pcaMap[BTN_A],        __cb_btn_a        )
    pca9555.attach    ( __pcaMap[BTN_B],        __cb_btn_b        )
    pca9555.attach    ( __pcaMap[BTN_START],    __cb_btn_start    )
    pca9555.attach    ( __pcaMap[BTN_SELECT],   __cb_btn_select   )
    pca9555.attach    ( __pcaMap[BTN_DOWN],     __cb_btn_down     )
    pca9555.attach    ( __pcaMap[BTN_RIGHT],    __cb_btn_right    )
    pca9555.attach    ( __pcaMap[BTN_UP],       __cb_btn_up       )
    pca9555.attach    ( __pcaMap[BTN_LEFT],     __cb_btn_left     )
    pca9555.attach    ( __pcaMap[BTN_HOME],     __cb_btn_home     )
    pca9555.attach    ( __pcaMap[BTN_MENU],     __cb_btn_menu     )
    pca9555.attach    ( __pcaMap[BTN_JOY],      __cb_btn_joy      )
    pca9555.attach    ( __pcaMap[PIN_CHARGING], __cb_pin_charging )
    pushMapping() # Add the initial / default mapping
    
# Public functions

def attach(button, callback):
    '''
    Attach a callback function to a button
    '''
    global __num, __cb
    if button < 0 or button >= __num:
        raise ValueError("Invalid button!")
    __cb[-1][button] = callback

def detach(button):
    '''
    Detach the callback function from a button
    '''
    global __num, __cb
    if button < 0 or button >= __num:
        raise ValueError("Invalid button!")
    __cb[-1][button] = None

def value(button):
    '''
    Read the state of a button (polling)
    '''
    global _buttons, __pcaMap, __num
    if button < 0 or button >= __num:
        raise ValueError("Invalid button!")
    return pca9555.value(__pcaMap[button])

def getCallback(button):
    '''
    Returns the currently attached callback function
    '''
    global __num, __cb
    if button < 0 or button >= __num:
        raise ValueError("Invalid button!")
    return __cb[-1][button]

def pushMapping(newMapping=None):
    '''
    Push a new button mapping on the stack
    '''
    global __cb
    if newMapping == None:
        newMapping = { BTN_UP: None, BTN_DOWN: None, BTN_LEFT: None, BTN_RIGHT: None, BTN_A: None, BTN_B: None, BTN_SELECT: None, BTN_START: None, BTN_HOME: None, BTN_MENU: None, BTN_JOY: None, PIN_CHARGING: None }
        if machine.nvs_getint("system", 'factory_checked'):
            newMapping[BTN_HOME] = __cbHome
            newMapping[BTN_MENU] = __cbMenu
    __cb.append(newMapping)

def popMapping():
    '''
    Pop the current button mapping from the stack
    '''
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

# ---
__init()
