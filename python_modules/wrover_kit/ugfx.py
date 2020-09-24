import display

activeList = None
listUpCallback = None
listDownCallback = None

BTN_A      = 0
BTN_B      = 1
BTN_START  = 2
BTN_SELECT = 3
JOY_DOWN   = 4
JOY_RIGHT  = 5
JOY_UP     = 6
JOY_LEFT   = 7

def input_attach(button, callback=None):
	global activeList, listUpCallback, listDownCallback
	if button < 0 or button > 7:
		return
	if button == JOY_UP:
		print("SET JOY UP LIST CB")
		listUpCallback = callback
	if button == JOY_DOWN:
		print("SET JOY DOWN LIST CB")
		listDownCallback = callback
	if not (button != JOY_UP or button != JOY_DOWN) or not activeList:
		print("MPR ATTACH",button)
		#mpr121.attach(button, callback)

LUT_NORMAL = 0
LUT_FASTER = 1
LUT_FULL   = 3
GREYSCALE = 900

BLACK = 0x000000
WHITE = 0xFFFFFF

def string(x,y,text,font,color):
	if font:
		display.font(font)
	_ = display.cursor(x,y)
	_ = display.textColor(color)
	display.print(text)

def width():
	return display.width()

def height():
	return display.height()

def display_image(x,y,data):
	display.png(x,y,data)

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

def init():
	pass

def input_init():
	pass

def set_lut(arg):
	pass

justifyLeft = 0
justifyCenter = 1
justifyRight = 2

def string_box(x,y,w,h,text,font,color,align):
	if font:
		display.font(font)
	if align == justifyRight:
		x = x + w - display.get_string_width(text)
	elif align == justifyCenter:
		x = x + int((w-display.get_string_width(text))/2)
	if x < 0:
		x = 0
	string(x,y,text,font,color)

class List():
	def __init__(self, x, y, w, h):
		self.x = x
		self.y = y
		self.w = w
		self.h = h
		self.items = []
		self.selected = 0
		global activeList
		activeList = self
		display.font("freesans9")
		self.lines = int(self.h / display.get_string_height(" "))
		self.offset = 0
		self.visible(True)
		self.enabled(True)
	
	def _draw(self):
		if self._visible:
			display.rect(self.x, self.y, self.w, self.h, True, 0xFFFFFF)
			display.rect(self.x, self.y, self.w, self.h, False, 0x000000)
			display.font("freesans9")
			_ = display.textColor(0x000000)
			display.cursor(self.x+1,self.y+1)
			totalHeight = 0
			for i in range(self.offset, len(self.items)):
				display.cursor(self.x+1,display.cursor()[1])
				item = self.items[i]
				lineHeight = display.get_string_height(item)
				totalHeight += lineHeight
				if totalHeight < self.h:
					if i == self.selected:
						display.rect(self.x, display.cursor()[1], self.w, lineHeight, True, 0x000000)
						_ = display.textColor(0xFFFFFF)
					else:
						_ = display.textColor(0x000000)
					display.cursor(self.x+1,display.cursor()[1]+3)
					display.print(item+"\n")
					display.cursor(self.x+1,display.cursor()[1]-3)
	
	def add_item(self, caption):
		i = self.items.append(caption)
		if self._enabled:
			self._draw()
		return i
	
	def count(self):
		return len(self.items)
	
	def remove_item(self, pos):
		self.items.pop(pos)
		if self._enabled:
			self._draw()
	
	def selected_index(self):
		return self.selected
	
	def destroy(self):
		self.items = []
		self.selected = 0
		activeList = None
		#mpr121.attach(JOY_UP, listUpCallback)
		#mpr121.attach(JOY_DOWN, listDownCallback)
		
	def _onUp(self, pressed):
		global listUpCallback
		if (pressed):
			if self.selected > 0:
				self.selected-=1
				if self.selected < self.offset:
					self.offset = self.selected
				self._draw()
		if listUpCallback:
			listUpCallback(pressed)
			
	def _onDown(self, pressed):
		global listDownCallback
		if (pressed):
			if self.selected < len(self.items):
				self.selected+=1
				if self.selected > self.offset+self.lines:
					self.offset = self.selected
				self._draw()
		if listDownCallback:
			listDownCallback(pressed)
		
	def visible(self, arg):
		self._visible = arg
		self._draw()

	def enabled(self, val):
		self._enabled = val
		global activeList, listUpCallback, listDownCallback
		if self._enabled:
			activeList = self
			#mpr121.attach(JOY_UP, self._onUp)
			#mpr121.attach(JOY_DOWN, self._onDown)
		else:
			activeList = None
			#mpr121.attach(JOY_UP, listUpCallback)
			#mpr121.attach(JOY_DOWN, listDownCallback)
		
def area(x,y,w,h,color):
	display.rect(x,y,w,h,True,color)
	
def rounded_box(x,y,w,h,r,color):
	display.rect(x,y,w,h,False,color)

def fill_rounded_box(x,y,w,h,r,color):
	display.rect(x,y,w,h,True,color)

def string_box(x,y,w,h,text,font,color,justify):
	display.textColor(color)
	display.font(font)
	if justify == justifyCenter:
		display.cursor(x+int(display.get_string_width(text)/2),y+int(display.get_string_height(text)/2))
	elif justify == justifyRight:
		display.cursor(x+display.get_string_width(text),y+int(display.get_string_height(text)/2))
	else:
		display.cursor(x,y+int(display.get_string_height(text)/2))
	display.print(text)
