import hub75, machine

PANEL_WIDTH = 32
PANEL_HEIGHT = 8
MAX_BRIGHTNESS = PANEL_WIDTH

MAX_BRIGHTNESS = PANEL_WIDTH

# Copies all attributes from hub75,
# so you can use e.g. rgb.scrolltext()
# instead of hub75.scrolltext()

for name in dir(hub75):
    globals()[name] = getattr(hub75, name)

def text(text, color=(255, 255, 255), pos=(0, 1)) :
    r, g, b = color
    x, y = pos
    hub75.text(text, r, g, b, x, y)

def scrolltext(text, color=(255, 255, 255), pos=(0, 1), width=PANEL_WIDTH):
    r, g, b = color
    x, y = pos
    hub75.scrolltext(text, r, g, b, x, y, width)

def background(color=(0, 0, 0)):
    r, g, b = color
    hub75.background(r, g, b)


def pixel(color=(255, 255, 255), pos=(0,0)):
    r, g, b = color
    x, y = pos
    hub75.pixel(r, g, b, x, y)


def get_brightness():
    return machine.nvs_getint('system', 'brightness')


def set_brightness(brightness=(MAX_BRIGHTNESS - 2)):
    brightness = 1 if brightness < 1 else (MAX_BRIGHTNESS if brightness > MAX_BRIGHTNESS else brightness)
    machine.nvs_setint('system', 'brightness', brightness)
    hub75.brightness(brightness)


# Restore previously set brightness
set_brightness(get_brightness() or (MAX_BRIGHTNESS - 2))