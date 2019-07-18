import display

def input_attach(button, callback):
	pass

BTN_START = -1
BTN_SELECT = -1
BTN_A = -1
BTN_B = -1
JOY_UP = -1
JOY_DOWN = -1
JOY_LEFT = -1
JOY_RIGHT = -1

LUT_NORMAL = 0
LUT_FASTER = 1
GREYSCALE = 900

BLACK = 0x000000
WHITE = 0xFFFFFF

def string(x,y,text,font,color):
	if font:
		display.font(font)
	yOffset = int(display.get_string_height(text) / 2)
	_ = display.cursor(x,y-yOffset)
	_ = display.textColor(color)
	display.print(text)

def width():
	return display.width()

def height():
	return display.height()

def display_image(x,y,data):
	pass
	#display.png(x,y,data)

def fill_circle(x, y, r, color):
	display.circle(x, y, r, 0, 359, True, color)

def circle(x, y, r, color):
	display.circle(x, y, r, 0, 359, False, color)

def clear(arg=None):
	if arg:
		display.fill(arg)
	else:
		display.fill()

def line(x0, y0, x1, y1, color):
	display.line(x0, y0, x1, y1, color)

def flush(arg=None):
	display.flush()
	
def get_string_width(arg, font=None):
	if font:
		display.font(font)
	return display.get_string_width(arg)

def get_string_height(arg, font=None):
	if font:
		display.font(font)
	return display.get_string_height(arg)

def input_init():
	pass

def set_lut(arg):
	pass

justifyLeft = 0
justifyRight = 1

def string_box(x,y,w,h,text,font,color,align):
	if font:
		display.font(font)
	if align == justifyRight:
		x = x + w - display.get_string_width(text)
	string(x,y,text,font,color)

class List():
	def __init__(self, x, y, w, h):
		self.x = x
		self.y = y
		self.w = w
		self.h = h
		self.items = []
		self.selected = 0
		pass
	
	def _draw(self):
		display.rect(self.x, self.y, self.w, self.h, True, 0xFFFFFF)
		display.rect(self.x, self.y, self.w, self.h, False, 0x000000)
		display.font("freesans9")
		_ = display.textColor(0x000000)
		display.cursor(self.x+1,self.y+1)
		totalHeight = 0
		for i in range(len(self.items)):
			display.cursor(self.x+1,display.cursor()[1])
			item = self.items[i]
			lineHeight = display.get_string_height(item)+8
			totalHeight += lineHeight
			if totalHeight < self.h:
				if i == self.selected:
					display.rect(self.x, display.cursor()[1], self.w, lineHeight, True, 0x000000)
					_ = display.textColor(0xFFFFFF)
				else:
					_ = display.textColor(0x000000)
				display.print(item+"\n")
		display.flush()
	
	def add_item(self, caption):
		i = self.items.append(caption)
		self._draw()
		return i
	
	def selected_index(self):
		return self.selected
	
	def destroy(self):
		self.items = []
		
def area(x,y,w,h,color):
	display.rect(x,y,w,h,color)
