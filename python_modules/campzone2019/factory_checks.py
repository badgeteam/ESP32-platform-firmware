import buttons, machine
import defines, rgb

import system
currentState = machine.nvs_getint('system', 'factory_checked') or 0
if currentState >= 2:
    machine.nvs_setint('system', 'factory_checked', 3)
    system.home()

RED     = (255, 0, 0)
GREEN   = (0, 255, 0)
BLUE    = (0, 0, 255)
WHITE   = (255, 255, 255)
BLACK   = (0, 0, 0)

CYAN   = (0, 255, 200)
YELLOW   = (255, 255, 0)

checklist = [
    (RED,   WHITE, 10, "UP",      defines.BTN_UP),
    (GREEN, WHITE, 4,  "DOWN",    defines.BTN_DOWN),
    (BLUE,  WHITE, 4,  "LEFT",    defines.BTN_LEFT),
    (BLACK, WHITE, 2,  "RIGHT",   defines.BTN_RIGHT),
    (BLACK, WHITE, 13, "A",       defines.BTN_A),
    (BLACK, WHITE, 13, "B",       defines.BTN_B),
]

def next_check():
    if(len(checklist) == 0):
        rgb.clear()
        rgb.background((0, 50, 0))
        rgb.text("Done!", CYAN, (4, 1))
        machine.nvs_setint('system', 'factory_checked', 3)
        return

    background, textcolor, x_pos, text, gpio = checklist.pop(0)
    rgb.clear()
    rgb.background(background)
    rgb.text(text, textcolor, (x_pos, 1))

    buttons.register(gpio, lambda pressed, gpio=gpio: (buttons.unassign(gpio), next_check()) if not pressed else None)

# Dim the screen so the factory checks can be done from USB only
rgb.brightness(5)
next_check()
