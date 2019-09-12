import display, buttons

mode = 0
xpos = 0

keyFont = "roboto_regular12"
textFont = "roboto_regular12"
charHeight = display.getTextHeight("X", keyFont)
yOffset = display.height() - charHeight
charWidth = display.getTextWidth("X", keyFont)

charMap = []
charMap.append(["a","b","c","d","e","f","g","h","i","j","k","l","m"])  # 0
charMap.append(["n","o","p","q","r","s","t","u","v","w","x","y","z"])  # 1
charMap.append(["A","B","C","D","E","F","G","H","I","J","K","L","M"])  # 2
charMap.append(["N","O","P","Q","R","S","T","U","V","W","X","Y","Z"])  # 3
charMap.append(["0","1","2","3","4","5","6","7","8","9"," ","@","#"])  # 4
charMap.append(["$","%","^","&","*","(",")",".",",","=","+","-","_"])  # 5
charMap.append(["[","]","?","\\","/","~","{","}","`","<",">",":",";"]) # 6

def onDown(pressed):
	global mode
	if pressed:
		mode += 1
		if mode > 8:
			mode = 0
		draw()

def onUp(pressed):
	global mode
	if pressed:
		mode -= 1
		if mode < 0:
			mode = 8
		draw()


def onRight(pressed):
	global xpos
	if pressed:
		if mode < 7:
			xpos += 1
			if xpos >= len(charMap[mode]):
				xpos = 0
		if mode == 7:
			cursorPos += 1
		draw()

def onLeft(pressed):
	global xpos
	if pressed:
		if mode < 7:
			xpos -= 1
			if xpos < 0:
				xpos = len(charMap[mode])-1
		if mode == 7:
			cursorPos += 1
		draw()

def onOk(pressed):
	global text, shift, cursorPos, xpos, mode, _cbAccept, _cbCancel
	if pressed:
		if mode == 0:
			if cy == 2 and cx == 0:
				shift += 1
				if shift > 4:
					shift = 0
			elif cy == 2 and cx == len(charMap[2]) - 1:
				# Backspace key
				if len(text) > 0:
					# text = text[:-1]
					if len(text) > cursorPos and cursorPos > 0:
						text = text[0 : cursorPos - 1 :] + text[cursorPos::]
						cursorPos -= 1
					else:
						text = text[:-1]
						cursorPos -= 1
			else:
				offset = shift * 3
				if cursorPos >= len(text):
					cursorPos += 1
				if charMap[cy + offset][cx] == "ENTER":
					text += "\n"
				else:
					text += charMap[cy + offset][cx]
				if shift == 1:
					shift = 0
			_correctLocation()
			draw()
		if mode == 2:
			buttons.popMapping()
			_cbAccept(text)


def onBack(pressed):
	global text, originalText, _cbAccept, _cbCancel
	if pressed and mode == 2:
		buttons.popMapping()
		if _cbCancel:
			_cbCancel(text)
		else:
			_cbAccept(originalText)


def draw():
	global cx, cy, text, cursorPos, title, yOffset, mode
	display.drawFill(0xFFFFFF)
	display.drawRect(0, yOffset - 12, display.width(), 12, True, 0x000000)
	display.drawRect(
		0, yOffset, display.width(), display.height() - yOffset, False, 0x000000
	)

	modeText = "keyboard"
	if mode == 1:
		modeText = "cursor"
	if mode == 2:
		modeText = "actions"

	display.drawText(0, yOffset - 12, "[SELECT] " + modeText, 0xFFFFFF, "Roboto_Regular12")
	display.drawRect(0, 0, display.width(), 14, True, 0)
	display.drawText(0, 0, title, 0xFFFFFF, "Roboto_Regular12")

	if cursorPos > len(text):
		cursorPos = len(text)
	if cursorPos < 0:
		cursorPos = 0

	textWithCursor = text[:cursorPos] + "|" + text[cursorPos:]

	textWithCursorLines = textWithCursor.split("\n")
	for i in range(len(textWithCursorLines)):
		display.drawText(0, 17 + i * 13, textWithCursorLines[i], 0x000000, textFont)

	for y in range(3):
		for x in range(10):
			xOffset = 0
			width = 29
			widthOffset = width
			if y == 1:
				xOffset = 6
			if y == 2 and x == 0:
				width *= 2
			if y == 2 and x == 1:
				continue
			if y == 1 and x == 0:
				width += xOffset
				xOffset = 0
			if y == 2 and x == 9:
				width += 6
			if x == 0 and y == 1 and shift == 4:
				width *= 2
			if x == 1 and y == 1 and shift == 4:
				width = 0
			selected = False
			if x == cx and y == cy:
				selected = True
			if width > 0:
				display.drawRect(
					x * widthOffset + xOffset,
					y * 20 + yOffset,
					width,
					20,
					True,
					0xFFFFFF,
				)
				display.drawRect(
					x * widthOffset + xOffset,
					y * 20 + yOffset,
					width,
					20,
					selected,
					0x000000,
				)
				color = 0
				if selected:
					color = 0xFFFFFF
				offset = shift * 3
				cxo = xOffset + charOffsetX
				# if x == 0 and y == 1:
				# 	cxo = xOffset+charOffsetXDouble
				display.drawText(
					x * widthOffset + cxo,
					y * 20 + yOffset + charOffsetY,
					charMap[y + offset][x],
					color,
					keyFont,
				)

	if mode == 2:
		display.drawRect(
			8,
			yOffset + 8,
			display.width() - 16,
			display.height() - yOffset - 16,
			True,
			0xFFFFFF,
		)
		display.drawRect(
			8,
			yOffset + 8,
			display.width() - 16,
			display.height() - yOffset - 16,
			False,
			0x000000,
		)
		display.drawText(
			12, yOffset + 12, "Press A to accept input", 0x000000, "Roboto_Regular12"
		)
		display.drawText(
			12, yOffset + 12 + 14, "Press B to cancel", 0x000000, "Roboto_Regular12"
		)

	display.flush(display.FLAG_LUT_FASTEST)


def show(newTitle, initialText, cbAccept, cbCancel=None):
	global cx, cy, text, originalText, title, cursorPos, _cbAccept, _cbCancel, mode
	mode = 0
	title = newTitle
	display.drawFill(0xFFFFFF)
	cx = 0
	cy = 0
	text = initialText
	originalText = initialText
	_cbAccept = cbAccept
	_cbCancel = cbCancel
	cursorPos = len(text)
	buttons.pushMapping();
	buttons.attach(buttons.BTN_A, onA)
	buttons.attach(buttons.BTN_B, onB)
	buttons.attach(buttons.BTN_SELECT, onSelect)
	buttons.attach(buttons.BTN_START, onStart)
	buttons.attach(buttons.BTN_DOWN, onDown)
	buttons.attach(buttons.BTN_RIGHT, onRight)
	buttons.attach(buttons.BTN_UP, onUp)
	buttons.attach(buttons.BTN_LEFT, onLeft)
	draw()
