import hid, keypad, display, time

display.drawFill(0x0000FF)
display.flush()
global fix
fix = False


def on_key(key_index, pressed):
    global fix
    print('key event')
    if(pressed):
        hid.keyboard_type("Cyber")

keypad.add_handler(on_key)