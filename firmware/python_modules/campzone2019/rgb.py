import hub75

PANEL_WIDTH = 32
PANEL_HEIGHT = 8

# Copies all attributes from hub75,
# so you can use e.g. rgb.scrolltext()
# instead of hub75.scrolltext()

for name in dir(hub75):
    globals()[name] = getattr(hub75, name)