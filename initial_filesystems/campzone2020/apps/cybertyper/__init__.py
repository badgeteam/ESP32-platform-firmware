import hid, keypad, display

display.drawFill(0x0000FF)
display.flush()
global fix
fix = False


def on_key(key_index, pressed):
    global fix
    x, y = key_index % 4, int(key_index / 4)
    print('key event')
    if(pressed):
        hid.keyboard_type("Cyber")
        display.drawPixel(x, y, 0xFF0000)
        display.flush()
    else:
        display.drawPixel(x, y, 0x0000FF) 
        display.flush()

keypad.add_handler(on_key)