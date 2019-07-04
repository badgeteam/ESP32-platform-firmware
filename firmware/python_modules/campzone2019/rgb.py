import hub75

PANEL_WIDTH = 32
PANEL_HEIGHT = 8

# Copies all attributes from hub75,
# so you can use e.g. rgb.scrolltext()
# instead of hub75.scrolltext()

for name in dir(hub75):
    globals()[name] = getattr(hub75, name)


def text(text, color=(255, 255, 255), pos=(0, 0)) :
    r, g, b = color
    x, y = pos
    hub75.scroll_text(text, r, g, b, x, y)

def scrolltext(text, color=(255, 255, 255), pos=(0, 0), width=PANEL_WIDTH):
    r, g, b = color
    x, y = pos
    hub75.scrolltext(text, r, g, b, x, y, width)

def background(color=(0, 0, 0)):
    r, g, b = color
    hub75.background(r, g, b)