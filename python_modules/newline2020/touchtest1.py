import machine, time, display, buttons, system

def home(pressed):
    if pressed:
        system.home()
buttons.attach(buttons.BTN_A, home)
buttons.attach(buttons.BTN_B, home)

display.orientation(90)
display.drawFill(0x000000)
display.flush()
display.backlight(255)

gpios = [  2,  12,  15,  13,  32,  33,  27]
names = ["U", "D", "L", "R", "A", "B", "C"]

thist = []
touch = []
for i in gpios:
    touch.append(machine.TouchPad(machine.Pin(i)))
    thist.append([0]*5)

while 1:
    line = 0
    display.drawFill(0x000000)
    display.drawRect(0,0,display.width()-1, 20, True, 0xFFFF00)
    display.drawText(0,0, "TOUCH DEMO", 0x000000, "ocra16")
    for i in touch:
        value = i.read()
        color = 0xAAAAAA
        touched = 0
        if value < 300:
            color = 0xFFFF00
            touched = 1
        _ = thist[line].pop(0)
        thist[line].append(touched)
        touchCount = 0
        for i in range(len(thist[line])):
            touchCount += thist[line][i]
        if touchCount > 2:
            color = 0x0000FF
        display.drawText(0,30+20*line, "{}: {:04d} {:01d}".format(names[line],value,touchCount), color, "ocra16")
        line += 1
    display.flush()
