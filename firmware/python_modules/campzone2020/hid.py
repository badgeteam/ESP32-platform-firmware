import time, stm32, keycodes

_OFFSET_I2C_USB_KEYBOARD = const(64)
_done_writing = True

_keyboard_queue = list()

def _keyboard_write():
    global _keyboard_queue
    if len(_keyboard_queue) == 0:
        return
    is_dirty = stm32.i2c_read_reg(_OFFSET_I2C_USB_KEYBOARD+7, 1)
    is_dirty = int.from_bytes(is_dirty, "little")
    if is_dirty:
        return
    payload = _keyboard_queue.pop(0)
    stm32.i2c_write_reg(_OFFSET_I2C_USB_KEYBOARD, payload)

def keyboard_press_keys(keys=b'\x00', modifier=b'\x00'):
    global _keyboard_queue

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
    _keyboard_queue.append(payload)
    _keyboard_write()

def keyboard_release():
    keyboard_press_keys(b'\x00\x00\x00\x00\x00\x00', b'\x00')

def keyboard_type(text):
    for character in text:
        keycode, shift = keycodes.char_to_keycode(character)
        modifier = bytes([keycodes.MOD_SHIFT]) if shift else b'\x00'
        keyboard_press_keys(bytes([keycode]), modifier)
    keyboard_release()

def _clear_writing_status():
    _keyboard_write()

stm32.add_interrupt_handler(stm32.INTERRUPT_HID_WRITTEN, _clear_writing_status)