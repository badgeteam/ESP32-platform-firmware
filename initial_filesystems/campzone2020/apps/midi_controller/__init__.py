import keypad, display, midi

display.drawFill(0xFF7F00)
display.flush()

def on_key(key_index, pressed):
    x, y = key_index % 4, int(key_index / 4)
    if pressed:
        print('Sending Note On', key_index)
        midi.note_on(midi.CENTRAL_C + key_index)
        display.drawPixel(x, y, 0xF8B700)
        display.flush()
    else:
        print('Sending Note Off', key_index)
        midi.note_off(midi.CENTRAL_C + key_index)
        display.drawPixel(x, y, 0xFF7F00)
        display.flush()

keypad.add_handler(on_key)