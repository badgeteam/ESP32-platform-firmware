import sys, version, badge

def goto(x,y):
	sys.stdout.write(u"\u001b["+str(y)+";"+str(x)+"H")

def home():
	goto(1,1)

def clear():
	sys.stdout.write(u"\u001b[2J")
	home()
	
def color(fg=37, bg=40, style=0):
	sys.stdout.write(u"\u001b["+str(style)+";"+str(fg)+";"+str(bg)+"m")
	
def header(cls = False, text = ""):
	if cls:
		clear()
	else:
		home()
	if text:
		text = "- "+text
	color(37, 44, 1)
	print(badge.deviceType.replace("_"," ")+" "+text+u"\r\n")
	color()
	
def draw_menu_item(text, selected, width=32):
	space = ""
	if width-len(text) > 0:
		space = " "*(width-len(text))
	if (selected):
		color(30, 47, 0)
		print("> " + text+space)
	else:
		color()
		print("  " + text+space)

def draw_menu(title, items, selected=0, text="", width=32):
	header(False, title)
	if len(text)>0:
		print(text)
		print("")
	for i in range(0, len(items)):
		draw_menu_item(items[i], selected==i, width)
	color()
	
def draw_menu_partial(title, items, selected=0, text="", width=32,lastSelected=0):
	if selected != lastSelected:
		goto(1,3+lastSelected)
		draw_menu_item(items[lastSelected], False, width)
		goto(1,3+selected)
		draw_menu_item(items[selected], True, width)
		color()
		
def menu(title, items, selected = 0, text="", width=32):
	clear()
	lastSelected = selected
	needFullDraw = True
	while True:
		if needFullDraw:
			draw_menu(title, items, selected, text, width)
		else:
			draw_menu_partial(title, items, selected, text, width, lastSelected)
		lastSelected = selected
		key = sys.stdin.read(1)
		feedPm()
		if (ord(key)==0x1b):
			key = sys.stdin.read(1)
			if (key=="["):
				key = sys.stdin.read(1)
				if (key=="A"):
					if (selected>0):
						selected -= 1
						needFullDraw = False
				if (key=="B"):
					if (selected<len(items)-1):
						selected += 1
						needFullDraw = False
		if (ord(key)==0x01):
			import tasks.powermanagement as pm, badge
			pm.disable()
			badge.rawrepl()
			draw_menu(title, items, selected, text)
			pm.resume()
			
		if (ord(key)==0xa):
			return selected

def prompt(prompt, x, y, buff = ""):
	running = True
	while running:
		goto(x, y)
		sys.stdout.write(prompt+": ")
		color(30, 47, 0)
		sys.stdout.write(buff)
		if len(buff) < 64:
			sys.stdout.write(" "*(64-len(buff)))
		color()
		last = sys.stdin.read(1)
		if last == '\n' or last == '\r':
			return buff
		if ord(last) >= 32 and ord(last) < 127:
			buff += last
		if ord(last) == 127:
			buff = buff[:-1]

def empty_lines(count = 10):
	for i in range(0,count):
		print("")

# Functions for feeding the power management task

powerManagement = None

def setPowerManagement(pm):
	global powerManagement
	powerManagement = pm
	
def feedPm():
	if powerManagement != None:
		powerManagement.set_timeout(300000) #Set timeout to 5 minutes
		powerManagement.feed()
