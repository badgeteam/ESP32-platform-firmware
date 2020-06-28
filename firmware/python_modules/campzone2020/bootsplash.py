import machine

machine.nvs_setint('system', 'splash_played', 1)

try:
    import display, audio, keypad, time, system

    audio.play('/cache/boot/splash.mp3')
    sleeptime = 0.25

    for _ in range(2):
        for i in range(7, 3, -1):
            display.drawFill(0x00)
            x, y = keypad.index_to_coords(i)
            display.drawPixel(x, y, 0x707070)
            x, y = keypad.index_to_coords(15-i)
            display.drawPixel(x, y, 0x707070)
            display.flush()
            time.sleep(sleeptime)

        for i in range(0, 4):
            display.drawFill(0x00)
            x, y = keypad.index_to_coords(i)
            display.drawPixel(x, y, 0x707070)
            x, y = keypad.index_to_coords(15-i)
            display.drawPixel(x, y, 0x707070)
            display.flush()
            time.sleep(sleeptime)

    for i in range(4):
        display.drawFill(0x00)
        display.drawPixel(0, i, 0x707070)
        display.drawPixel(1, i, 0x707070)
        display.drawPixel(2, 3-i, 0x707070)
        display.drawPixel(3, 3-i, 0x707070)
        display.flush()
        time.sleep(sleeptime)

    for i in range(4):
        display.drawFill(0x00)
        display.drawPixel(i, 0, 0x707070)
        display.drawPixel(i, 1, 0x707070)
        display.drawPixel(3-i, 2, 0x707070)
        display.drawPixel(3-i, 3, 0x707070)
        display.flush()
        time.sleep(sleeptime)

    for i in range(4):
        display.drawFill(0x00)
        display.drawPixel(i, 0, 0x707070)
        display.drawPixel(3-i, 3, 0x707070)
        display.flush()
        time.sleep(sleeptime)

    for i in range(1, 4):
        display.drawFill(0x00)
        display.drawPixel(3, i, 0x707070)
        display.drawPixel(0, 3-i, 0x707070)
        display.flush()
        time.sleep(sleeptime)

    for i in range(150):
        display.drawFill(0x00)
        for x in range(1,3):
            for y in range(1,3):
                display.drawPixel(x, y, (i << 16) + (i << 8) + i)
        display.flush()
        time.sleep(0.04)
except:
    print('Exception in boot splash')
    import system
    system.crashedWarning()

system.reboot()