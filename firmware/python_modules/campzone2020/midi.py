import time, stm32, keycodes

_OFFSET_I2C_USB_MIDI = const(78)
_done_writing = True

def _write_midi_packet(data):
    if type(data) is bytes and len(data) != 3:
        print('MIDI packet must be exactly 3 bytes long')
        return

    for slot in range(4):
        address = _OFFSET_I2C_USB_MIDI + slot * 4
        dirty_byte = address + 3
        is_dirty = stm32.i2c_read_reg(dirty_byte, 1)
        if not is_dirty:
            stm32.i2c_write_reg(address, data + b'0x01')
            return

    # No free slots were found
    while not _done_writing:
        time.sleep(0.01)


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

def _clear_writing_status():
    global _done_writing
    _done_writing = True

stm32.add_interrupt_handler(stm32.INTERRUPT_MIDI_WRITTEN, _clear_writing_status)