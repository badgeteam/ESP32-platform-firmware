import display as _display
import buttons as _buttons

_MODE           = 0
_XPOS           =  0
_LASTPARTMOVEUP = False
_VISIBLELINE    = 0
_FONT_KEY       = "roboto_regular12"
_FONT_TEXT      = "roboto_regular12"
_CHAR_HEIGHT    = _display.getTextHeight("X", _FONT_KEY)
_CHAR_WIDTH     = _display.getTextWidth("X", _FONT_KEY)

_CHAR_MAP = [
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

def _calcPos():
	global _TEXT, _CURSOR_X, _CURSOR_Y
	parts = _TEXT.split("\n")
	pos = 0
	for i in range(_CURSOR_Y):
		pos += len(parts[i])+1
	pos += _CURSOR_X
	if len(parts[_CURSOR_Y]) < 1:
		_CURSOR_X = 0
	return pos

def _changePosition(amount):
	global _XPOS
	_XPOS += amount
	if _XPOS >= len(_CHAR_MAP):
		_XPOS = _XPOS - len(_CHAR_MAP)
	if _XPOS < 0:
		_XPOS = len(_CHAR_MAP) + _XPOS

def _onUp(pressed):
	global _MODE, _CURSOR_X, _CURSOR_Y, _TEXT, _LASTPARTMOVEUP
	if pressed:
		if _MODE == 0:
			_changePosition(9)
		if _MODE == 1:
			_CURSOR_Y -= 1
			if _CURSOR_Y < 0:
				_CURSOR_Y = 0
			if _CURSOR_X > len(_TEXT.split("\n")[_CURSOR_Y]):
				_CURSOR_X = len(_TEXT.split("\n")[_CURSOR_Y])
		_draw()

def _onDown(pressed):
	global _MODE, _CURSOR_X, _CURSOR_Y, _TEXT, _LASTPARTMOVEUP
	if pressed:
		if _MODE == 0:
			_changePosition(-9)
		if _MODE == 1:
			_CURSOR_Y += 1
			if _CURSOR_Y > len(_TEXT.split("\n"))-1:
				_CURSOR_Y = len(_TEXT.split("\n"))-1
			if _CURSOR_X > len(_TEXT.split("\n")[_CURSOR_Y]):
				_CURSOR_X = len(_TEXT.split("\n")[_CURSOR_Y])
		_draw()

def _onRight(pressed):
	global _XPOS, _CURSOR_X, _CURSOR_Y, _TEXT, _MODE
	if pressed:
		if _MODE == 0:
			_changePosition(1)
		if _MODE == 1:
			_CURSOR_X += 1
			if _CURSOR_X > len(_TEXT.split("\n")[_CURSOR_Y]):
				_CURSOR_Y += 1
				if _CURSOR_Y > len(_TEXT.split("\n"))-1:
					_CURSOR_Y = len(_TEXT.split("\n"))-1
					_CURSOR_X = len(_TEXT.split("\n")[_CURSOR_Y])
				else:
					_CURSOR_X = 0
		_draw()

def _onLeft(pressed):
	global _XPOS, _CURSOR_X, _CURSOR_Y, _TEXT, _MODE
	if pressed:
		if _MODE == 0:
			_changePosition(-1)
		if _MODE == 1:
			if _CURSOR_X > 0 or _CURSOR_Y > 0:
				_CURSOR_X -= 1
				if _CURSOR_X < 0:
					_CURSOR_Y -= 1
					if _CURSOR_Y < 0:
						_CURSOR_Y = 0
					_CURSOR_X = len(_TEXT.split("\n")[_CURSOR_Y])
		_draw()
def _onA(pressed):
	global _TEXT, shift, _CURSOR_X, _CURSOR_Y, _XPOS, _MODE, _CB_ACCEPT, _CB_CANCEL
	if pressed:
		if _MODE == 0:
			_TEXTPos = _calcPos()
			if _TEXTPos >= len(_TEXT):
				_TEXT += _CHAR_MAP[_XPOS]
			else:
				_TEXT = _TEXT[:_TEXTPos] + _CHAR_MAP[_XPOS] + _TEXT[_TEXTPos:]
			_CURSOR_X += 1
			_draw()
		if _MODE == 1:
			_TEXTPos = _calcPos()
			if _TEXTPos >= len(_TEXT):
				_TEXT += "\n"
			else:
				_TEXT = _TEXT[:_TEXTPos] + "\n" + _TEXT[_TEXTPos:]
			_CURSOR_X = 0
			_CURSOR_Y += 1
			_draw()
		if _MODE == 2:
			_MODE = 3
			_draw()
			_buttons.popMapping()
			_CB_ACCEPT(_TEXT)

def _onB(pressed):
	global _TEXT, _TEXT_ORIG, _CB_ACCEPT, _CB_CANCEL, _CURSOR_X, _CURSOR_Y, _MODE
	if pressed:
		if _MODE == 2:
			_MODE = 3
			_draw()
			_buttons.popMapping()
			if _CB_CANCEL:
				_CB_CANCEL(_TEXT)
			else:
				_CB_ACCEPT(_TEXT_ORIG)
		else:
			if len(_TEXT) > 0:
				_TEXTPos = _calcPos()
				lenOfPrevLine = 0
				if len(_TEXT.split("\n"))>0 and _CURSOR_Y > 0:
					lenOfPrevLine = len(_TEXT.split("\n")[_CURSOR_Y-1])
				if len(_TEXT) > _TEXTPos and _TEXTPos > 0:
					_TEXT = _TEXT[0 : _TEXTPos - 1 :] + _TEXT[_TEXTPos::]
					_CURSOR_X -= 1
				elif len(_TEXT) > 0:
					_TEXT = _TEXT[:-1]
					_CURSOR_X -= 1
				if _CURSOR_X < 0:
					_CURSOR_X = lenOfPrevLine
					_CURSOR_Y -= 1
					if _CURSOR_Y < 0:
						_CURSOR_Y = 0
		_draw()

def _onSelect(pressed):
	global _MODE
	if pressed:
		_MODE += 1
		if _MODE > 2:
			_MODE = 0
		_draw()

def _draw():
	global _TEXT, _CURSOR_X, _CURSOR_Y, _TITLE, _MODE, _XPOS, _LASTPARTMOVEUP, _VISIBLELINE

	_MODE_TEXT = "INP"
	if _MODE == 1:
		_MODE_TEXT = "CUR"
	if _MODE == 2:
		_MODE_TEXT = "ACT"
		
	_display.drawFill(0xFFFFFF)
	_display.drawRect(0, 0, _display.width(), 14, True, 0x000000)
	_display.drawText(0, 0, _TITLE, 0xFFFFFF, _FONT_KEY)
	_display.drawText(128-24, 0, _MODE_TEXT, 0xFFFFFF, _FONT_KEY)

	offsetX1 = 0
	arrow1 = False
	offsetX2 = 0
	arrow2 = False

	cursorPos = _calcPos()

	_TEXTWithCursor = _TEXT[:cursorPos] + "|" + _TEXT[cursorPos:]

	_TEXTWithCursorLines = _TEXTWithCursor.split("\n")
	_TEXTWithCursorLines.append("")
	
	lineWithCursorLength = _display.getTextWidth(_TEXTWithCursorLines[_CURSOR_Y], _FONT_TEXT)
	lineWithCursorLengthUntilCursor = _display.getTextWidth(_TEXTWithCursorLines[_CURSOR_Y][:_CURSOR_X], _FONT_TEXT)
	
	if (lineWithCursorLength < _display.width()):
		offsetX = 0
	else:
		offsetX = lineWithCursorLengthUntilCursor + _display.getTextWidth(" ", _FONT_TEXT) - _display.width()//2
		if offsetX < -_display.getTextWidth(" ", _FONT_TEXT):
			offsetX = -_display.getTextWidth(" ", _FONT_TEXT)
		
	arrow = lineWithCursorLength - offsetX > _display.width()
	
	if _VISIBLELINE + 1 < _CURSOR_Y:
		_VISIBLELINE = _CURSOR_Y -1
	
	if _VISIBLELINE < 0:
		_VISIBLELINE = 0
	
	if _CURSOR_Y < _VISIBLELINE:
		_VISIBLELINE = _CURSOR_Y
		
	for i in range(len(_TEXTWithCursorLines)):
		v = "-"
		if i == _VISIBLELINE:
			v = ">"
	
	arrow1L = False
	arrow2L = False
	
	if _CURSOR_Y == _VISIBLELINE:
		offsetX1 = offsetX
		if offsetX > 0:
			 arrow1L = True
		if arrow:
			arrow1 = True
	else:
		if _display.getTextWidth(_TEXTWithCursorLines[_VISIBLELINE], _FONT_TEXT) > _display.width():
			arrow1 = True
	if _CURSOR_Y == _VISIBLELINE+1:
		offsetX2 = offsetX
		if offsetX > 0:
			 arrow2L = True
		if arrow:
			arrow2 = True
	else:
		if _display.getTextWidth(_TEXTWithCursorLines[_VISIBLELINE+1], _FONT_TEXT) > _display.width():
			arrow2 = True
	
	_display.drawText(-offsetX1, 17 + 0 * 13, _TEXTWithCursorLines[_VISIBLELINE], 0x000000, _FONT_TEXT)
	_display.drawText(-offsetX2, 17 + 1 * 13, _TEXTWithCursorLines[_VISIBLELINE+1], 0x000000, _FONT_TEXT)
	
	if arrow1:
		_display.drawRect(119, 17 + 0 * 13, 9, _display.getTextHeight(" ", _FONT_TEXT), True, 0x000000)
		_display.drawText(120, 17 + 0 * 13, ">", 0xFFFFFF, _FONT_TEXT)
	
	if arrow2:
		_display.drawRect(119, 17 + 1 * 13, 9, _display.getTextHeight(" ", _FONT_TEXT), True, 0x000000)
		_display.drawText(120, 17 + 1 * 13, ">", 0xFFFFFF, _FONT_TEXT)

	if arrow1L:
		_display.drawRect(0, 17 + 0 * 13, 9, _display.getTextHeight(" ", _FONT_TEXT), True, 0x000000)
		_display.drawText(0, 17 + 0 * 13, "<", 0xFFFFFF, _FONT_TEXT)
	
	if arrow2L:
		_display.drawRect(0, 17 + 1 * 13, 9, _display.getTextHeight(" ", _FONT_TEXT), True, 0x000000)
		_display.drawText(0, 17 + 1 * 13, "<", 0xFFFFFF, _FONT_TEXT)

	_display.drawRect(0, _display.height()-15, _display.width(), 15, True, 0x000000)
	if _MODE == 0:
		for i in range(9):
			item = _XPOS + (i-4)
			if item < 0:
				item = len(_CHAR_MAP)+item
			if item >= len(_CHAR_MAP):
				item = item % len(_CHAR_MAP)
			if _XPOS == item:
				_display.drawRect(i*14+1, _display.height()-14, 14, 14, True, 0xFFFFFF)
				_display.drawRect(i*14+1, _display.height()-14, 14, 14, False, 0x000000)
				_display.drawText(i*14+1 + 3, _display.height()-14, _CHAR_MAP[item], 0x000000, _FONT_KEY)
			else:
				_display.drawRect(i*14+1, _display.height()-14, 14, 14, True, 0x000000)
				_display.drawText(i*14+1 + 3, _display.height()-14, _CHAR_MAP[item], 0xFFFFFF, _FONT_KEY)
	elif _MODE == 1:
		_display.drawText(0, _display.height()-14, "CURSOR, A: NL, B: RM", 0xFFFFFF, _FONT_KEY)
	elif _MODE == 2:
		_display.drawText(0, _display.height()-14, "A: OK, B: CANCEL", 0xFFFFFF, _FONT_KEY)
	else:
		_display.drawText(0, _display.height()-14, "Processing...", 0xFFFFFF, _FONT_KEY)

	_display.flush(_display.FLAG_LUT_FASTEST)


def show(new_TITLE, initial_TEXT, cbAccept, cbCancel=None):
	'''
	Show a _TEXT input dialog to the user
	:param _TITLE: _TITLE of the dialog
	:param _TEXT: Intial value (can be "")
	:param cbAccept: Callback executed when the user accepts
	:param cbCancel: (optional) Callback executed when the user cancels
	'''
	global _TEXT, _TEXT_ORIG, _TITLE, _CURSOR_X, _CURSOR_Y, _CB_ACCEPT, _CB_CANCEL, _MODE, _LASTPARTMOVEUP
	_MODE = 0
	_TITLE = new_TITLE
	_display.drawFill(0xFFFFFF)
	_TEXT = initial_TEXT
	_TEXT_ORIG = initial_TEXT
	_CB_ACCEPT = cbAccept
	_CB_CANCEL = cbCancel
	_CURSOR_Y = len(_TEXT.split("\n"))-1
	_CURSOR_X = len(_TEXT.split("\n")[-1])+1
	_LASTPARTMOVEUP = True
	_VISIBLELINE = 0
	_buttons.pushMapping();
	_buttons.attach(_buttons.BTN_A, _onA)
	_buttons.attach(_buttons.BTN_B, _onB)
	_buttons.attach(_buttons.BTN_DOWN, _onDown)
	_buttons.attach(_buttons.BTN_RIGHT, _onRight)
	_buttons.attach(_buttons.BTN_UP, _onUp)
	_buttons.attach(_buttons.BTN_LEFT, _onLeft)
	_buttons.attach(_buttons.BTN_SELECT, _onSelect)
	_draw()
