import stm32

_OFFSET_I2C_USB_KEYBOARD = const(64)
_done_writing = false

def send_keyboard_keys(keys, modifier=b'\x00'):
    if type(keys) is not bytes:
        raise Exception('Keycodes must be passed as bytes')
    if len(keys) > 6:
        raise Exception('Maximally 6 keycodes may be sent over USB at once')
    if type(modifier) is not bytes or len(modifier) != 1:
        raise Exception('Key modifier must be bytes of length 1')

    # Pad keycodes with zeroes until we have exactly 6 keycodes
    keys = keys + (bytes([0] * (6 - len(keys))))
    dirty_byte = b'\x01'
    payload = modifier + keys + dirty_byte
    stm32.i2c_write_reg(_OFFSET_I2C_USB_KEYBOARD, payload)