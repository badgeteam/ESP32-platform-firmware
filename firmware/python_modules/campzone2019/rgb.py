import hub75, machine

PANEL_WIDTH = 32
PANEL_HEIGHT = 8
MAX_BRIGHTNESS = PANEL_WIDTH

MAX_BRIGHTNESS = PANEL_WIDTH

FONT_7x5 = 0
FONT_6x3 = 1

current_font = FONT_7x5
font_heights = [7, 6]

# Copies all attributes from hub75,
# so you can use e.g. rgb.scrolltext()
# instead of hub75.scrolltext()

for name in dir(hub75):
    globals()[name] = getattr(hub75, name)

def text(text, color=(255, 255, 255), pos=None):
    if pos is None:
        pos = (0, int((8 - font_heights[current_font]) / 2))

    r, g, b = color
    x, y = pos
    hub75.text(text, r, g, b, x, y)

def scrolltext(text, color=(255, 255, 255), pos=None, width=PANEL_WIDTH):
    if pos is None:
        pos = (0, int((8 - font_heights[current_font]) / 2))

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


def gif(data, pos=(0,0), size=(8,8), frames=1):
    x, y = pos
    size_x, size_y = size
    hub75.gif(data, x, y, size_x, size_y, frames)


def image(data, pos=(0,0), size=(8,8)):
    x, y = pos
    size_x, size_y = size
    hub75.image(data, x, y, size_x, size_y)


def getbrightness():
    return machine.nvs_getint('system', 'brightness')


def setbrightness(brightness=(MAX_BRIGHTNESS - 2)):
    brightness = 1 if brightness < 1 else (MAX_BRIGHTNESS if brightness > MAX_BRIGHTNESS else brightness)
    hub75.brightness(brightness)

    # Allow non-visible brightness, but don't make it persistent across reboot
    if brightness >= 3:
        machine.nvs_setint('system', 'brightness', brightness)

def setfont(font_index):
    global current_font
    if font_index < 0 or font_index >= 2:
        return

    current_font = font_index
    hub75.setfont(font_index)


# Restore previously set brightness
setbrightness(getbrightness() or (MAX_BRIGHTNESS - 2))