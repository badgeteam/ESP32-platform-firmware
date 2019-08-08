import display, mpr121

keyFont  = "freesans9"
textFont = "roboto_regular12"
yOffset = 128-20*3

shift = 0 #0 = lower, 1 = upper, 2 = caps, 3 = numbers, 4 = symbols
mode  = 0 #0 = keyboard, 1 = select, 2 = buttons

display.drawFill(0xFFFFFF)

charWidth  = display.getTextWidth("X", keyFont)
charHeight = display.getTextHeight("X", keyFont)

charOffsetX = int((20-charWidth))
charOffsetXDouble = int((20-charWidth*1.5))
charOffsetY = int((20-charHeight))+3

charMap = []
charMap.append(['q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'])
charMap.append([' ', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l'])
charMap.append(['SHIFT', ' ', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<<'])
# Uppercase
charMap.append(['Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'])
charMap.append([' ', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L'])
charMap.append(['CAPS', ' ', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<<'])
# Caps-lock
charMap.append(['Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'])
charMap.append([' ', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L'])
charMap.append(['1 2 3', ' ', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<<'])
# Numbers
charMap.append(['1', '2', '3', '4', '5', '6', '7', '8', '9', '0'])
charMap.append([' ', '@', '#', '$', '%', '^', '&', '*', '(', ')'])
charMap.append(['!@#$', ' ', '.', ',', '=', '+', '[', ']', '?', '<<'])
# Numbers 2
charMap.append(['{', '}', ';', ':', '\"', '\'', ',', '.', '<', '>'])
charMap.append([' ', '~', '`', '/', '\\', ':)', ':(', ' ', ' ', ' '])
charMap.append(['A B C', ' ', '-', '_', ' ', ' ', ' ', ' ', ' ', '<<'])

cx = 0
cy = 0
text = ""

def _correctLocation(right=False):
	global cx, cy
	if cx < 0:
		cx = 0
	if cy < 0:
		cy = 0
	if cy >= 3:
		cy = 2
	if cx >= len(charMap[cy]):
		cx = len(charMap[cy])-1
	if cy==2 and cx == 1:
		if right:
			cx = 2
		else:
			cx = 0
	print(cx, cy)

def onSelect(pressed):
	if pressed:
		global mode
		mode+=1
		if mode > 2:
			mode = 0

def onDown(pressed):
	if pressed:
		global cy
		cy += 1
		_correctLocation()
		draw()

def onRight(pressed):
	if pressed:
		global cx
		cx += 1
		_correctLocation(True)
		draw()

def onUp(pressed):
	if pressed:
		global cy
		cy -= 1
		_correctLocation()
		draw()

def onLeft(pressed):
	if pressed:
		global cx
		cx -= 1
		_correctLocation()
		draw()
		
def onA(pressed):
	global text, shift
	if pressed:
		if cy == 2 and cx == 0:
			shift += 1
			if shift > 4:
				shift = 0
		elif cy == 2 and cx == len(charMap[2])-1:
			#Backspace key
			if len(text) > 0:
				text = text[:-1]
		else:
			offset = shift*3
			text += charMap[cy+offset][cx]
			if shift == 1:
				shift = 0
		_correctLocation()
		draw()

def draw():
	global cx, cy, text
	display.drawFill(0xFFFFFF)
	display.drawText(5, 5, text, 0x000000, textFont)
	for y in range(3):
		for x in range(10):
			xOffset = 0
			width = 29
			if y == 1:
				xOffset = 6
			if y == 2 and x == 0:
				width *= 2
			if y == 2 and x == 1:
				continue
			selected = False
			if x == cx and y == cy:
				selected = True
			display.drawRect(x*29+xOffset, y*20+yOffset, width, 20, selected, 0x000000)
			color = 0
			if selected:
				color = 0xFFFFFF
			offset = shift*3
			cxo = xOffset+charOffsetX
			#if x == 0 and y == 1:
			#	cxo = xOffset+charOffsetXDouble
			display.drawText(x*29+cxo, y*20+yOffset+charOffsetY, charMap[y+offset][x], color, keyFont)
		
	display.flush(display.FLAG_LUT_FASTEST)

mpr121.attach(0, onA)
mpr121.attach(3, onSelect)
mpr121.attach(4, onDown)
mpr121.attach(5, onRight)
mpr121.attach(6, onUp)
mpr121.attach(7, onLeft)
draw()
