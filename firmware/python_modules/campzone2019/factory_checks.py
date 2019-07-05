import buttons
import defines, rgb

RED     = (255, 0, 0)
GREEN   = (0, 255, 0)
BLUE    = (0, 0, 255)
WHITE   = (255, 255, 255)
BLACK   = (0, 0, 0)

CYAN   = (0, 255, 200)
YELLOW   = (255, 255, 0)

checklist = [
    (RED,   WHITE, "UP",      defines.BTN_UP),
    (GREEN, WHITE, "DOWN",    defines.BTN_DOWN),
    (BLUE,  WHITE, "LEFT",    defines.BTN_LEFT),
    (BLACK, WHITE, "RIGHT",   defines.BTN_RIGHT),
    (BLACK, WHITE, "A",       defines.BTN_A),
    (BLACK, WHITE, "B",       defines.BTN_B),
]

def next_check():
    if(len(checklist) == 0):
        rgb.background(CYAN)
        rgb.text("Done!", YELLOW, (12, 1))
        return

    background, textcolor, text, gpio = checklist.pop()
    rgb.background(background)
    rgb.text(text, textcolor, (12, 1))

    buttons.register(gpio, lambda pressed, gpio=gpio: (buttons.unassign(gpio), next_check()) if not pressed else None)

next_check()