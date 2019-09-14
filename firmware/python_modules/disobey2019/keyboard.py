import display, buttons

mode = 0
xpos = 4

keyFont = "roboto_regular12"
textFont = "roboto_regular12"
charHeight = display.getTextHeight("X", keyFont)
charWidth = display.getTextWidth("X", keyFont)

charMap = []
charMap.append(["a","b","c","d","e","f","g","h","i","j","k","l","m"])  # 0
charMap.append(["n","o","p","q","r","s","t","u","v","w","x","y","z"])  # 1
charMap.append(["A","B","C","D","E","F","G","H","I","J","K","L","M"])  # 2
charMap.append(["N","O","P","Q","R","S","T","U","V","W","X","Y","Z"])  # 3
charMap.append(["0","1","2","3","4","5","6","7","8","9"," ","@","#"])  # 4
charMap.append(["$","%","^","&","*","(",")",".",",","=","+","-","_"])  # 5
charMap.append(["[","]","?","\\","/","~","{","}","`","<",">",":",";"]) # 6

def onUp(pressed):
	global mode
	if pressed:
		mode += 1
		if mode > 8:
			mode = 0
		print("Mode increased",mode)
		draw()

def onDown(pressed):
	global mode
	if pressed:
		mode -= 1
		if mode < 0:
			mode = 8
		print("Mode decreased",mode)
		draw()


def onRight(pressed):
	global xpos, cursorPos
	if pressed:
		if mode < 7:
			xpos += 1
			if xpos >= len(charMap[mode]):
				xpos = 0
		if mode == 7:
			cursorPos += 1
		draw()

def onLeft(pressed):
	global xpos, cursorPos
	if pressed:
		if mode < 7:
			xpos -= 1
			if xpos < 0:
				xpos = len(charMap[mode])-1
		if mode == 7:
			cursorPos += 1
		draw()

				# Backspace key
				#if len(text) > 0:
				#	# text = text[:-1]
				#	if len(text) > cursorPos and cursorPos > 0:
				#		text = text[0 : cursorPos - 1 :] + text[cursorPos::]
				#		cursorPos -= 1
				#	else:
				#		text = text[:-1]
				#		cursorPos -= 1

			#buttons.popMapping()
			#_cbAccept(text)

def onOk(pressed):
	global text, shift, cursorPos, xpos, mode, _cbAccept, _cbCancel
	if pressed:
		text += charMap[mode][xpos]
		cursorPos += 1
		draw()



def onBack(pressed):
	global text, originalText, _cbAccept, _cbCancel
	if pressed and mode == 2:
		buttons.popMapping()
		if _cbCancel:
			_cbCancel(text)
		else:
			_cbAccept(originalText)


def draw():
	global cx, cy, text, cursorPos, title, mode, xpos
	display.drawFill(0xFFFFFF)
	display.drawRect(0, 0, display.width(), 14, True, 0x000000)
	display.drawText(0, 0, title, 0xFFFFFF, keyFont)

	modeText = "keyboard"
	if mode == 1:
		modeText = "cursor"
	if mode == 2:
		modeText = "actions"

	if cursorPos > len(text):
		cursorPos = len(text)
	if cursorPos < 0:
		cursorPos = 0

	textWithCursor = text[:cursorPos] + "|" + text[cursorPos:]

	textWithCursorLines = textWithCursor.split("\n")
	for i in range(len(textWithCursorLines)):
		display.drawText(0, 17 + i * 13, textWithCursorLines[i], 0x000000, textFont)

	offset = xpos
	if offset < 4:
		offset = 4
	if offset > (len(charMap[mode])-4):
		offset = len(charMap[mode])-4-9

	if mode < 7:
		for i in range(9):
			item = (offset-4+i)
			print(item, len(charMap[mode]), xpos)
			if xpos == item:
				display.drawRect(i*14, display.height()-14, 14, 14, True, 0x000000)
				display.drawText(i*14 + 3, display.height()-14, charMap[mode][item], 0xFFFFFF, keyFont)
			else:
				display.drawRect(i*14, display.height()-14, 14, 14, False, 0x000000)
				display.drawText(i*14 + 3, display.height()-14, charMap[mode][item], 0x000000, keyFont)

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
	buttons.attach(buttons.BTN_OK, onOk)
	buttons.attach(buttons.BTN_BACK, onBack)
	buttons.attach(buttons.BTN_DOWN, onDown)
	buttons.attach(buttons.BTN_RIGHT, onRight)
	buttons.attach(buttons.BTN_UP, onUp)
	buttons.attach(buttons.BTN_LEFT, onLeft)
	draw()
