import display, mpr121, gc

display_flags = 0

def orientation(deg=None):
	if deg:
		display.orientation(deg)
	else:
		return display.orientation()

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
	doAttach = True
	if button == JOY_UP:
		listUpCallback = callback
		if activeList:
			doAttach = False
	elif button == JOY_DOWN:
		listDownCallback = callback
		if activeList:
			doAttach = False
	if doAttach:
		mpr121.attach(button, callback)

LUT_NORMAL = display.FLAG_LUT_NORMAL
LUT_FASTER = display.FLAG_LUT_FAST
LUT_FASTEST = display.FLAG_LUT_FASTEST
LUT_FULL   = display.FLAG_FULL
GREYSCALE = display.FLAG_LUT_GREYSCALE

BLACK = 0x000000
WHITE = 0xFFFFFF

def string(x,y,text,font,color):
	if not color:
		color = 0
	if font:
		display.drawText(x, y, text, color, font)
	else:
		display.drawText(x, y, text, color)

def width():
	return display.width()

def height():
	return display.height()

def display_image(x,y,data):
	display.drawPng(x,y,data)

def fill_circle(x, y, r, color):
	display.drawCircle(x, y, r, 0, 359, True, color)

def circle(x, y, r, color):
	display.drawCircle(x, y, r, 0, 359, False, color)

def clear(arg=None):
	if arg:
		display.drawFill(arg)
	else:
		display.drawFill()

def line(x0, y0, x1, y1, color):
	display.drawLine(x0, y0, x1, y1, color)

def flush(arg=None):
	global display_flags
	if arg == None and display_flags == 0:
		display.flush(display.FLAG_LUT_FAST)
	else:
		display.flush(display_flags)
	
def get_string_width(text, font=None):
	if font:
		return display.getTextWidth(text, font)
	else:
		return display.getTextWidth(text)

def get_string_height(text, font=None):
	if font:
		return display.getTextHeight(text, font)
	else:
		return display.getTextHeight(text)

def init():
	print("This app uses the UGFX compatibility layer, using UGFX is deprecated, please check the wiki for details!")

def input_init():
	pass

def set_lut(arg):
	display_flags = arg

justifyLeft = 0
justifyCenter = 1
justifyRight = 2
		
def area(x,y,w,h,color):
	display.drawRect(x,y,w,h,True,color)
	
def rounded_box(x,y,w,h,r,color):
	display.drawLine(x+r,   y,     x+w-1-r, y,       color)
	display.drawLine(x+r,   y+h-1, x+w-1-r, y+h-1,   color)
	display.drawLine(x,     y+r,   x,       y+h-1-r, color)
	display.drawLine(x+w-1, y+r,   x+w-1,   y+h-1-r, color)
	display.drawCircle(x+r,     y+r,     r, 270, 359, False, color)
	display.drawCircle(x+w-1-r, y+r,     r, 0,   90,  False, color)
	display.drawCircle(x+r,     y+h-1-r, r, 180, 270, False, color)
	display.drawCircle(x+w-1-r, y+h-1-r, r, 90,  180, False, color)
	#display.drawRect(x,y,w,h,False,color)

def fill_rounded_box(x,y,w,h,r,color):
	display.drawRect(x,y,w,h,True,color)

def string_box(x,y,w,h,text,font,color,align):
	textWidth = 0
	if font:
		textWidth = display.getTextWidth(text, font)
	else:
		textWidth = display.getTextWidth(text)
	if align == justifyRight:
		x = x + w - textWidth
	elif align == justifyCenter:
		x = x + int((w-textWidth)/2)
	if x < 0:
		x = 0
	display.drawText(x, y, text, color, font)


# Listbox UI element
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
		self.lines = int(self.h / display.getTextHeight(" ", "freesans9"))
		self.offset = 0
		self.visible(True)
		self.enabled(True)
	
	def _draw(self):
		if self._visible:
			display.drawRect(self.x, self.y, self.w, self.h, True, 0xFFFFFF)
			display.drawRect(self.x, self.y, self.w, self.h, False, 0x000000)
			cursor = (self.x+1,self.y+1)
			totalHeight = 0
			for i in range(len(self.items)-self.offset):
				cursor = (self.x+1,cursor[1])
				item = self.items[self.offset+i]
				lineHeight = display.getTextHeight(item, "freesans9")
				
				while display.getTextWidth(item, "freesans9") > self.w:
					item = item[:-1]
				
				totalHeight += lineHeight
				if totalHeight >= self.h:
					break
				color = 0x000000
				if self.offset+i == self.selected:
					display.drawRect(self.x, cursor[1], self.w, lineHeight, True, 0x000000)
					color = 0xFFFFFF
				cursor = (self.x+1,cursor[1]+3)
				display.drawText(cursor[0], cursor[1], item+"\n", color, "freesans9")
				cursor = (self.x+1,cursor[1]-3+display.getTextHeight(item+"\n", "freesans9"))
	
	def add_item(self, caption):
		if type(caption) == type(""):
			i = self.items.append(caption)
		elif type(caption) == type(b""):
			i = self.items.append(caption.decode('utf-8'))
		else:
			i = self.items.append(str(caption))
		if self._enabled:
			self._draw()
		return i
	
	def count(self):
		return len(self.items)
	
	def selected_text(self):
		return self.items[self.selected]
	
	def remove_item(self, pos):
		self.items.pop(pos)
		if self._enabled:
			self._draw()
			
	def clear(self):
		self.selected = 0
		self.items = []
		gc.collect()
	
	def selected_index(self, setValue=None):
		if setValue:
			self.selected = setValue
			self._draw()
		else:
			return self.selected
	
	def destroy(self):
		self.items = []
		self.selected = 0
		activeList = None
		mpr121.attach(JOY_UP, listUpCallback)
		mpr121.attach(JOY_DOWN, listDownCallback)
		
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
			if self.selected < len(self.items)-1:
				self.selected+=1
				if self.selected >= self.offset+self.lines:
					self.offset += 1
				#print("onDown", self.selected, len(self.items), self.offset, self.items[self.selected])
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
			mpr121.attach(JOY_UP, self._onUp)
			mpr121.attach(JOY_DOWN, self._onDown)
		else:
			activeList = None
			mpr121.attach(JOY_UP, listUpCallback)
			mpr121.attach(JOY_DOWN, listDownCallback)
