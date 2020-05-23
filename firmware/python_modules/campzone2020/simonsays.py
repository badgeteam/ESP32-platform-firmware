import system, display, keypad, machine, sndmixer

off = 0x000000
green_bright = 0x00FF00
green_dark = 0x002000
red_bright = 0xFF0000
red_dark = 0x200000
blue_bright = 0x0000FF
blue_dark = 0x000020
yellow_bright = 0xFFFF00
yellow_dark = 0x202000

#top-left = green color
display.drawPixel(0, 0, green_dark)
display.drawPixel(1, 0, green_dark)
display.drawPixel(0, 1, green_dark)
display.drawPixel(1, 1, green_dark)
#top-right = red color
display.drawPixel(2, 0, red_dark)
display.drawPixel(3, 0, red_dark)
display.drawPixel(2, 1, red_dark)
display.drawPixel(3, 1, red_dark)
#bottom-right = blue color
display.drawPixel(2, 2, blue_dark)
display.drawPixel(3, 2, blue_dark)
display.drawPixel(2, 3, blue_dark)
display.drawPixel(3, 3, blue_dark)
#bottom-left = yellow color
display.drawPixel(0, 2, yellow_dark)
display.drawPixel(1, 2, yellow_dark)
display.drawPixel(0, 3, yellow_dark)
display.drawPixel(1, 3, yellow_dark)
display.flush()

sndmixer.begin(4, True)

def on_key(key_index, pressed):
    x, y = key_index % 4, int(key_index / 4)
    if pressed:
        #top-left = green color
        if (x == 0 or x == 1) and (y == 0 or y == 1):
            display.drawPixel(0, 0, green_bright)
            display.drawPixel(1, 0, green_bright)
            display.drawPixel(0, 1, green_bright)
            display.drawPixel(1, 1, green_bright)
        #top-right = red color
        elif(x == 2 or x == 3) and (y == 0 or y == 1):
            display.drawPixel(2, 0, red_bright)
            display.drawPixel(3, 0, red_bright)
            display.drawPixel(2, 1, red_bright)
            display.drawPixel(3, 1, red_bright)
        #bottom-right = blue color
        elif(x == 2 or x == 3) and (y == 2 or y == 3):
            display.drawPixel(2, 2, blue_bright)
            display.drawPixel(3, 2, blue_bright)
            display.drawPixel(2, 3, blue_bright)
            display.drawPixel(3, 3, blue_bright)
        #bottom-left = yellow color
        elif(x == 0 or x == 1) and (y == 2 or y == 3):
            display.drawPixel(0, 2, yellow_bright)
            display.drawPixel(1, 2, yellow_bright)
            display.drawPixel(0, 3, yellow_bright)
            display.drawPixel(1, 3, yellow_bright)
        display.flush()
    else:
        #top-left = green color
        display.drawPixel(0, 0, green_dark)
        display.drawPixel(1, 0, green_dark)
        display.drawPixel(0, 1, green_dark)
        display.drawPixel(1, 1, green_dark)
        #top-right = red color
        display.drawPixel(2, 0, red_dark)
        display.drawPixel(3, 0, red_dark)
        display.drawPixel(2, 1, red_dark)
        display.drawPixel(3, 1, red_dark)
        #bottom-right = blue color
        display.drawPixel(2, 2, blue_dark)
        display.drawPixel(3, 2, blue_dark)
        display.drawPixel(2, 3, blue_dark)
        display.drawPixel(3, 3, blue_dark)
        #bottom-left = yellow color
        display.drawPixel(0, 2, yellow_dark)
        display.drawPixel(1, 2, yellow_dark)
        display.drawPixel(0, 3, yellow_dark)
        display.drawPixel(1, 3, yellow_dark)
        display.flush()

keypad.add_handler(on_key)