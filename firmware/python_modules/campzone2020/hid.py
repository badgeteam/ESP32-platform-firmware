import time, stm32, keycodes

_OFFSET_I2C_USB_KEYBOARD = const(64)
_done_writing = True

def keyboard_press_keys(keys=b'\x00', modifier=b'\x00'):
    global _done_writing
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
    _done_writing = False
    stm32.i2c_write_reg(_OFFSET_I2C_USB_KEYBOARD, payload)
    while not _done_writing:
        time.sleep(0.01)

def keyboard_release():
    keyboard_press_keys(b'\x00\x00\x00\x00\x00\x00', b'\x00')

def keyboard_type(text):
    for character in text:
        keycode, shift = keycodes.char_to_keycode(character)
        modifier = keycodes.SHIFT if shift else b'\x00'
        keyboard_press_keys(bytes([keycode]), modifier)
    keyboard_release()

def _clear_writing_status():
    global _done_writing
    _done_writing = True

stm32.add_interrupt_handler(stm32.INTERRUPT_HID_WRITTEN, _clear_writing_status)