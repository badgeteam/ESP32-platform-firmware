import hid, keypad, display

display.drawFill(0x0000FF)
display.flush()

def on_key(key_index, pressed):
    print('key event')
    hid.keyboard_type('Typing\n')

keypad.add_handler(on_key)