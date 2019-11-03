import display, buttons

mode = 0
xpos = 0
lastPartMoveUp = False
visibleLine = 0

keyFont = "roboto_regular12"
textFont = "roboto_regular12"
charHeight = display.getTextHeight("X", keyFont)
charWidth = display.getTextWidth("X", keyFont)

charMap = [
	"a","b","c","d","e","f","g","h","i",
	"j","k","l","m","n","o","p","q","r",
	"s","t","u","v","w","x","y","z"," ",
	"A","B","C","D","E","F","G","H","I",
	"J","K","L","M","N","O","P","Q","R",
	"S","T","U","V","W","X","Y","Z"," ",
	"0","1","2","3","4","5","6","7","8",
	"9"," ","@","#","$","%","^","&","*",
	"(",")",".",",","=","+","-","_","[",
	"]","?","\\","/","~","{","}","`","<",
	">",":",";"
	]

def calcPos():
	global text, cursorX, cursorY
	parts = text.split("\n")
	pos = 0
	for i in range(cursorY):
		pos += len(parts[i])+1
	pos += cursorX
	if len(parts[cursorY]) < 1:
		cursorX = 0
	print("Cursor ( ",cursorX,",",cursorY," ) = ",pos)
	return pos

def changePosition(amount):
	global xpos
	xpos += amount
	if xpos >= len(charMap):
		xpos = xpos - len(charMap)
	if xpos < 0:
		xpos = len(charMap) + xpos

def onUp(pressed):
	global mode, cursorX, cursorY, text, lastPartMoveUp
	if pressed:
		if mode == 0:
			changePosition(9)
		if mode == 1:
			cursorY -= 1
			if cursorY < 0:
				cursorY = 0
			if cursorX > len(text.split("\n")[cursorY]):
				cursorX = len(text.split("\n")[cursorY])
		draw()

def onDown(pressed):
	global mode, cursorX, cursorY, text, lastPartMoveUp
	if pressed:
		if mode == 0:
			changePosition(-9)
		if mode == 1:
			cursorY += 1
			if cursorY > len(text.split("\n"))-1:
				cursorY = len(text.split("\n"))-1
			if cursorX > len(text.split("\n")[cursorY]):
				cursorX = len(text.split("\n")[cursorY])
		draw()

def onRight(pressed):
	global xpos, cursorX, cursorY, text, mode
	if pressed:
		if mode == 0:
			changePosition(1)
		if mode == 1:
			cursorX += 1
			if cursorX > len(text.split("\n")[cursorY]):
				cursorY += 1
				if cursorY > len(text.split("\n"))-1:
					cursorY = len(text.split("\n"))-1
					cursorX = len(text.split("\n")[cursorY])
				else:
					cursorX = 0
		draw()

def onLeft(pressed):
	global xpos, cursorX, cursorY, text, mode
	if pressed:
		if mode == 0:
			changePosition(-1)
		if mode == 1:
			if cursorX > 0 or cursorY > 0:
				cursorX -= 1
				if cursorX < 0:
					cursorY -= 1
					if cursorY < 0:
						cursorY = 0
					cursorX = len(text.split("\n")[cursorY])
		draw()
def onA(pressed):
	global text, shift, cursorX, cursorY, xpos, mode, _cbAccept, _cbCancel
	if pressed:
		if mode == 0:
			textPos = calcPos()
			if textPos >= len(text):
				text += charMap[xpos]
			else:
				text = text[:textPos] + charMap[xpos] + text[textPos:]
			cursorX += 1
			draw()
		if mode == 1:
			textPos = calcPos()
			if textPos >= len(text):
				text += "\n"
			else:
				text = text[:textPos] + "\n" + text[textPos:]
			cursorX = 0
			cursorY += 1
			draw()
		if mode == 2:
			mode = 3
			draw()
			buttons.popMapping()
			_cbAccept(text)

def onB(pressed):
	global text, originalText, _cbAccept, _cbCancel, cursorX, cursorY, mode
	if pressed:
		if mode == 2:
			mode = 3
			draw()
			buttons.popMapping()
			if _cbCancel:
				_cbCancel(text)
			else:
				_cbAccept(originalText)
		else:
			if len(text) > 0:
				textPos = calcPos()
				lenOfPrevLine = 0
				if len(text.split("\n"))>0 and cursorY > 0:
					lenOfPrevLine = len(text.split("\n")[cursorY-1])
				if len(text) > textPos and textPos > 0:
					text = text[0 : textPos - 1 :] + text[textPos::]
					cursorX -= 1
				elif len(text) > 0:
					text = text[:-1]
					cursorX -= 1
				if cursorX < 0:
					cursorX = lenOfPrevLine
					cursorY -= 1
					if cursorY < 0:
						cursorY = 0
		draw()

def onSelect(pressed):
	global mode
	if pressed:
		mode += 1
		if mode > 2:
			mode = 0
		draw()

def draw():
	global cx, cy, text, cursorX, cursorY, title, mode, xpos, lastPartMoveUp, visibleLine

	modeText = "INP"
	if mode == 1:
		modeText = "CUR"
	if mode == 2:
		modeText = "ACT"
		
	display.drawFill(0xFFFFFF)
	display.drawRect(0, 0, display.width(), 14, True, 0x000000)
	display.drawText(0, 0, title, 0xFFFFFF, keyFont)
	display.drawText(128-24, 0, modeText, 0xFFFFFF, keyFont)

	offsetX1 = 0
	arrow1 = False
	offsetX2 = 0
	arrow2 = False

	cursorPos = calcPos()

	textWithCursor = text[:cursorPos] + "|" + text[cursorPos:]

	textWithCursorLines = textWithCursor.split("\n")
	textWithCursorLines.append("")
	
	lineWithCursorLength = display.getTextWidth(textWithCursorLines[cursorY], textFont)
	lineWithCursorLengthUntilCursor = display.getTextWidth(textWithCursorLines[cursorY][:cursorX], textFont)
	
	if (lineWithCursorLength < display.width()):
		offsetX = 0
	else:
		offsetX = lineWithCursorLengthUntilCursor + display.getTextWidth(" ", textFont) - display.width()//2
		if offsetX < -display.getTextWidth(" ", textFont):
			offsetX = -display.getTextWidth(" ", textFont)
		print("Applied X offset", offsetX)
		
	arrow = lineWithCursorLength - offsetX > display.width()
	
	if visibleLine + 1 < cursorY:
		visibleLine = cursorY -1
		print("Cursor was below visible area")
	
	if visibleLine < 0:
		visibleLine = 0
		print("Corrected visible area to line 0")
	
	if cursorY < visibleLine:
		visibleLine = cursorY
		print("Cursor was above visible area")
	
	print("Drawing line (and +1)", visibleLine)
	
	for i in range(len(textWithCursorLines)):
		v = "-"
		if i == visibleLine:
			v = ">"
		print(v+" "+textWithCursorLines[i])
	
	arrow1L = False
	arrow2L = False
	
	if cursorY == visibleLine:
		offsetX1 = offsetX
		if offsetX > 0:
			 arrow1L = True
		if arrow:
			arrow1 = True
	else:
		if display.getTextWidth(textWithCursorLines[visibleLine], textFont) > display.width():
			arrow1 = True
	if cursorY == visibleLine+1:
		offsetX2 = offsetX
		if offsetX > 0:
			 arrow2L = True
		if arrow:
			arrow2 = True
	else:
		if display.getTextWidth(textWithCursorLines[visibleLine+1], textFont) > display.width():
			arrow2 = True
	
	display.drawText(-offsetX1, 17 + 0 * 13, textWithCursorLines[visibleLine], 0x000000, textFont)
	display.drawText(-offsetX2, 17 + 1 * 13, textWithCursorLines[visibleLine+1], 0x000000, textFont)
	
	if arrow1:
		display.drawRect(119, 17 + 0 * 13, 9, display.getTextHeight(" ", textFont), True, 0x000000)
		display.drawText(120, 17 + 0 * 13, ">", 0xFFFFFF, textFont)
	
	if arrow2:
		display.drawRect(119, 17 + 1 * 13, 9, display.getTextHeight(" ", textFont), True, 0x000000)
		display.drawText(120, 17 + 1 * 13, ">", 0xFFFFFF, textFont)

	if arrow1L:
		display.drawRect(0, 17 + 0 * 13, 9, display.getTextHeight(" ", textFont), True, 0x000000)
		display.drawText(0, 17 + 0 * 13, "<", 0xFFFFFF, textFont)
	
	if arrow2L:
		display.drawRect(0, 17 + 1 * 13, 9, display.getTextHeight(" ", textFont), True, 0x000000)
		display.drawText(0, 17 + 1 * 13, "<", 0xFFFFFF, textFont)

	display.drawRect(0, display.height()-15, display.width(), 15, True, 0x000000)
	if mode == 0:
		for i in range(9):
			item = xpos + (i-4)
			if item < 0:
				item = len(charMap)+item
			if item >= len(charMap):
				item = item % len(charMap)
			if xpos == item:
				display.drawRect(i*14+1, display.height()-14, 14, 14, True, 0xFFFFFF)
				display.drawRect(i*14+1, display.height()-14, 14, 14, False, 0x000000)
				display.drawText(i*14+1 + 3, display.height()-14, charMap[item], 0x000000, keyFont)
			else:
				display.drawRect(i*14+1, display.height()-14, 14, 14, True, 0x000000)
				display.drawText(i*14+1 + 3, display.height()-14, charMap[item], 0xFFFFFF, keyFont)
	elif mode == 1:
		display.drawText(0, display.height()-14, "CURSOR, A: NL, B: RM", 0xFFFFFF, keyFont)
	elif mode == 2:
		display.drawText(0, display.height()-14, "A: OK, B: CANCEL", 0xFFFFFF, keyFont)
	else:
		display.drawText(0, display.height()-14, "Processing...", 0xFFFFFF, keyFont)

	display.flush(display.FLAG_LUT_FASTEST)


def show(newTitle, initialText, cbAccept, cbCancel=None):
	global cx, cy, text, originalText, title, cursorX, cursorY, _cbAccept, _cbCancel, mode, lastPartMoveUp
	mode = 0
	title = newTitle
	display.drawFill(0xFFFFFF)
	cx = 0
	cy = 0
	text = initialText
	originalText = initialText
	_cbAccept = cbAccept
	_cbCancel = cbCancel
	cursorY = len(text.split("\n"))-1
	cursorX = len(text.split("\n")[-1])+1
	lastPartMoveUp = True
	visibleLine = 0
	buttons.pushMapping();
	buttons.attach(buttons.BTN_A, onA)
	buttons.attach(buttons.BTN_B, onB)
	buttons.attach(buttons.BTN_DOWN, onDown)
	buttons.attach(buttons.BTN_RIGHT, onRight)
	buttons.attach(buttons.BTN_UP, onUp)
	buttons.attach(buttons.BTN_LEFT, onLeft)
	buttons.attach(buttons.BTN_SELECT, onSelect)
	draw()
