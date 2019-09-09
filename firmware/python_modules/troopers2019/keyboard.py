import display, buttons, _thread, time

__mapping = {
	#              (default, shift, shield, fn)
	buttons.KEY_A: ('a', 'A', None, None),
	buttons.KEY_B: ('b', 'B', ',', '<'),
	buttons.KEY_C: ('c', 'C', '-', '_'),
	buttons.KEY_D: ('d', 'D', None, None),
	buttons.KEY_E: ('e', 'E', '3', '#'),
	buttons.KEY_F: ('f', 'F', None, None),
	buttons.KEY_G: ('g', 'G', ';', ':'),
	buttons.KEY_H: ('h', 'H', '\'', '"'),
	buttons.KEY_I: ('i', 'I', '8', '*'),
	buttons.KEY_J: ('j', 'J', '[', '{'),
	buttons.KEY_K: ('k', 'K', ']', '}'),
	buttons.KEY_L: ('l', 'L', '\\', '|'),
	buttons.KEY_M: ('m', 'M', '/', '?'),
	buttons.KEY_N: ('n', 'N', '.', '>'),
	buttons.KEY_O: ('o', 'O', '9', '('),
	buttons.KEY_P: ('p', 'P', '0', ')'),
	buttons.KEY_Q: ('q', 'Q', '1', '!'),
	buttons.KEY_R: ('r', 'R', '4', '$'),
	buttons.KEY_S: ('s', 'S', None, None),
	buttons.KEY_T: ('t', 'T', '5', '%'),
	buttons.KEY_U: ('u', 'U', '7', '&'),
	buttons.KEY_V: ('v', 'V', '=', '+'),
	buttons.KEY_W: ('w', 'W', '2', '@'),
	buttons.KEY_X: ('x', 'X', None, None),
	buttons.KEY_Y: ('y', 'Y', '6', '^'),
	buttons.KEY_Z: ('z', 'Z', None, None),
	buttons.KEY_SPACE: (' ', None, None, None),
	buttons.KEY_RETURN: ('\n', None, None, None),
}

textFont = "roboto_regular18"

def __onSelect(pressed):
	if pressed:
		draw()

def __onStart(pressed):
	pass

def __onDown(pressed):
	global cursorPos
	if pressed:
		draw()

def __onRight(pressed):
	global cursorPos
	if pressed:
		cursorPos += 1
		draw()


def __onUp(pressed):
	global cursorPos
	if pressed:
		draw()

def __onShift(pressed):
	global mode
	if pressed:
		mode = 1
	else:
		mode = 0

def __onShield(pressed):
	global mode
	if pressed:
		mode = 2
	else:
		mode = 0

def __onFn(pressed):
	global mode
	if pressed:
		mode = 3
	else:
		mode = 0

def __onLeft(pressed):
	global cursorPos
	if pressed:
		cursorPos -= 1
		draw()

def __onBackspace(pressed):
	global text, cursorPos
	if pressed and len(text) > 0:
			if len(text) > cursorPos and cursorPos > 0:
				text = text[0 : cursorPos - 1 :] + text[cursorPos::]
				cursorPos -= 1
			else:
				text = text[:-1]
				cursorPos -= 1
	draw()

def __onInput(key, pressed):
	global text, mode, cursorPos
	if pressed:
		text = text[0 : cursorPos :] + __mapping[key][mode] + text[cursorPos::]
		cursorPos += 1
	draw()

def __onA(pressed):
	global text, active
	if pressed:
			buttons.popMapping()
			active = False
			_cbAccept(text)


def __onB(pressed):
	global text, originalText, _cbAccept, _cbCancel, active
	if pressed:
		buttons.popMapping()
		active = False
		if _cbCancel:
			_cbCancel(text)
		else:
			_cbAccept(originalText)


def __drawThread():
	global text, cursorPos, title, __drawChanged, active
	while active:
		if __drawChanged:
			__drawChanged = False
			display.drawFill(0xFFFFFF)
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

			display.flush(display.FLAG_LUT_FASTEST)
		time.sleep(0.1)

def draw():
	global __drawChanged
	__drawChanged = True

active = False
def show(newTitle, initialText, cbAccept, cbCancel=None):
	global cx, cy, text, originalText, title, cursorPos, _cbAccept, _cbCancel, mode, active, __drawChanged
	if not active:
		active = True
		mode = 0
		title = newTitle
		display.drawFill(0xFFFFFF)
		cx = 0
		cy = 0
		text = initialText
		originalText = initialText
		_cbAccept = cbAccept
		_cbCancel = cbCancel
		__drawChanged = True
		cursorPos = len(text)
		buttons.pushMapping()
		buttons.attach(buttons.BTN_A, __onA)
		buttons.attach(buttons.BTN_B, __onB)
		buttons.attach(buttons.BTN_SELECT, __onSelect)
		buttons.attach(buttons.BTN_START, __onStart)
		buttons.attach(buttons.BTN_DOWN, __onDown)
		buttons.attach(buttons.BTN_RIGHT, __onRight)
		buttons.attach(buttons.BTN_UP, __onUp)
		buttons.attach(buttons.BTN_LEFT, __onLeft)
		buttons.attach(buttons.KEY_BACKSPACE, __onBackspace)
		buttons.attach(buttons.KEY_SHIFT, __onShift)
		buttons.attach(buttons.KEY_SHIELD, __onShield)
		buttons.attach(buttons.KEY_FN, __onFn)
		buttons.attach(buttons.KEY_ANY, __onInput)
		draw()
		_thread.start_new_thread("threadDraw",__drawThread, ())
