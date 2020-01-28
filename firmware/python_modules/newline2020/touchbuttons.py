import machine, time, _thread

# --- Thresholds ---
_pressed = 100  # Range observed (Tom): < 60 when pressed, > 500 when not pressed. Tweak this threshold if there are false positives.
_num_debounces = 1  # Set this to a larger (whole) number to decrease false positives (but increase latency)
_monitor_running = False

# --- BUTTON CONSTANTS  ---
BTN_UP     = 0
BTN_DOWN   = 1
BTN_LEFT   = 2
BTN_RIGHT  = 3
BTN_A      = 4
BTN_B      = 5
BTN_START  = 6

# --- GPIOs ---
gpios = [2, 12, 15, 13, 32, 33, 27]

# --- Pin Initialisation ---
pins = [machine.TouchPad(gpio) for gpio in gpios]
states = [False] * len(pins)
debounces = [_num_debounces + 1] * len(pins) # Not set to 0 because then the callbacks would trigger once enough debounce measurements are taken

# --- Callbacks for when press states change ---
callbacks = [None] * len(pins)

def is_pressed(button):
    pin = pins[button]
    try:
        state = pin.read()
    except:
        state = False

    return pin.read() < _pressed

def set_callback(button, callback_function):
    callbacks[button] = callback_function

def _touchMonitor():
    _monitor_running = True
    while True:
        for button,_ in enumerate(pins):
            state = is_pressed(button)
            if state != states[button]:
                debounces[button] = 1
                states[button] = state
            else:
                debounces[button] += 1

            if debounces[button] == _num_debounces:
                try:
                    callbacks[button](state)
                except:
                    pass
        time.sleep(0.01)

if not _monitor_running:
    _thread.start_new_thread('TouchMonitor', _touchMonitor, ())