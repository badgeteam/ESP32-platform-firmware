import machine, keypad, display, audio

background = 0x050505
was_pressed = [False] * 4

display.drawFill(background)
display.flush()


def on_key(index, pressed):
    x, y = keypad.index_to_coords(index)
    if not x == y:
        return
    colour = [0xFF0000, 0x00FF00, 0x0000FF, 0xFF00FF][x]
    display.drawPixel(x, y, colour if pressed else background)
    if pressed:
        was_pressed[x] = True
        if was_pressed == [True] * 4:
            machine.nvs_setint('system', 'factory_checked', 2)
            audio.play('/cache/system/works.mp3')
            display.drawFill(0x007F00)
    display.flush()


keypad.add_handler(on_key)
