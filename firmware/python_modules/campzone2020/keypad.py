import stm32

_N_KEYS = const(16)
_OFFSET_I2C_KEY_STATE = const(4)

keypad_state = [0] * _N_KEYS
keypad_handlers = []

def add_handler(handler):
    global keypad_handlers
    keypad_handlers.append(handler)

def remove_handler(handler):
    global keypad_handlers
    for index, _handler in enumerate(keypad_handlers):
        if handler == _handler:
            del keypad_handlers[index]

def get_state():
    return keypad_state

def index_to_coords(key_index):
    x, y = key_index % 4, int(key_index / 4)
    return x, y

def _get_key_state():
    response = stm32.i2c_read_reg(_OFFSET_I2C_KEY_STATE, 2)
    buttons = bin(int.from_bytes(response, 'little'))[2:]
    buttons = '0' * (_N_KEYS-len(buttons)) + buttons
    state = [char == '1' for char in buttons]
    state.reverse()
    return state

def _keypad_interrupt_handler():
    global keypad_state
    new_touch_state = _get_key_state()
    for index, new_state in enumerate(new_touch_state):
        if keypad_state[index] != new_state:
            for handler in keypad_handlers:
                try:
                    handler(index, new_state)
                except Exception as e:
                    import sys
                    print('Exception in keypad event handler')
                    sys.print_exception(e)
    keypad_state = new_touch_state

stm32.add_interrupt_handler(stm32.INTERRUPT_KEYPAD, _keypad_interrupt_handler)