import keypad, display, midi

display.drawFill(0x00FF00)
display.flush()

def on_key(key_index, pressed):
    if pressed:
        print('Sending Note On', key_index)
        midi.note_on(midi.CENTRAL_C + key_index)
    else:
        print('Sending Note Off', key_index)
        midi.note_off(midi.CENTRAL_C + key_index)

keypad.add_handler(on_key)