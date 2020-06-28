import touchbuttons

## TODO: look up correct value for each buttons
LEFT = 128
RIGHT = 256
HOME = 512
CANCEL = 64
OK = 16

wildcard_handlers = []
touchpad_handlers = {LEFT: None, RIGHT: None, HOME: None, CANCEL: None, OK: None}
touchpad_state = {LEFT: False, RIGHT: False, HOME: False, CANCEL: False, OK: False}

def on(key, handler):
    global touchpad_handlers
    if key not in touchpad_handlers:
        print('Invalid key given to touchpad.on()')
        return
    touchpad_handlers[key] = handler

def off(key):
    global touchpad_handlers
    if key not in touchpad_handlers:
        print('Invalid key given to touchpad.on()')
        return
    touchpad_handlers[key] = None

def add_handler(handler):
    global wildcard_handlers
    wildcard_handlers.append(handler)

def remove_handler(handler):
    global wildcard_handlers
    for index, _handler in wildcard_handlers:
        if handler == _handler:
            del wildcard_handlers[index]

def get_state():
    return touchpad_state

def _on_touch(state):
    for key, handler in touchpad_handlers.items():
        is_pressed = state & key
        if touchpad_state[key] != is_pressed and handler is not None:
            handler(is_pressed)
        touchpad_state[key] = is_pressed

touchbuttons.set_handler(_on_touch)