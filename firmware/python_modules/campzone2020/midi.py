import time, stm32, keycodes

_OFFSET_I2C_USB_MIDI = const(78)
_done_writing = True

CENTRAL_C = 60 # 60th note number in the MIDI protocol

_note_queue = list()

def _write_midi_packet():
    global _note_queue    
    if len(_note_queue) == 0:
        return
    
    for slot in range(4):
        address = _OFFSET_I2C_USB_MIDI + slot * 4
        dirty_byte = address + 3
        is_dirty = stm32.i2c_read_reg(dirty_byte, 1)
        is_dirty = int.from_bytes(is_dirty, "little")
        if not is_dirty:
            data = note_queue.pop(0)
            if type(data) is bytes and len(data) != 3:
                print('MIDI packet must be exactly 3 bytes long')
                return
            stm32.i2c_write_reg(address, data + b'0x01')
            return

def _clear_writing_status():
    _write_midi_packet()

def note_on(note_number, velocity=127, midi_channel=0):
    """
    See https://www.inspiredacoustics.com/en/MIDI_note_numbers_and_center_frequencies for note numbers.
    """
    global _note_queue
    payload = bytes([0x90 + midi_channel, note_number, velocity])
    _note_queue.append(payload)
    _write_midi_packet()

def note_off(note_number, velocity=127, midi_channel=0):
    """
    See https://www.inspiredacoustics.com/en/MIDI_note_numbers_and_center_frequencies for note numbers.
    """
    global _note_queue
    payload = bytes([0x80 + midi_channel, note_number, velocity])
    _note_queue.append(payload)
    _write_midi_packet()


stm32.add_interrupt_handler(stm32.INTERRUPT_MIDI_WRITTEN, _clear_writing_status)