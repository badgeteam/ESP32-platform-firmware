import display, badge, version, time, orientation, machine

if orientation.isLandscape():
	NUM_LINES = 6
else:
	NUM_LINES = 18

# Functions
def msg_nosplit(message, title = 'Loading...', reset = False):
	global NUM_LINES
	"""Show a terminal style loading screen with title

	title can be optionaly set when resetting or first call
	"""
	global messageHistory
		
	try:
		messageHistory
		if reset:
			raise exception
	except:
		display.drawFill(0xFFFFFF)
		display.drawText(0, 0, title, 0x000000, version.font_header)
		h = display.getTextHeight(" ", version.font_header)
		display.drawLine(0, h, display.width(), h, 0x000000)
		messageHistory = []

	lineHeight = int(display.getTextHeight(" ", version.font_default) / 2)+5

	if len(messageHistory)<NUM_LINES:
		display.drawText(0, 28 + (len(messageHistory) * lineHeight), message, 0x000000, version.font_default)
		messageHistory.append(message)
	else:
		messageHistory.pop(0)
		messageHistory.append(message)
		display.drawRect(0, 15, display.width(), display.height()-15, True, 0xFFFFFF)
		for i, message in enumerate(messageHistory):
			display.drawText(0, 28 + (i * lineHeight), message, 0x000000, version.font_default)

	display.flush(display.FLAG_LUT_FASTEST)
	
def msg(message, title = "Loading...", reset = False, wait = 0):
	global NUM_LINES
	try:
		lines = lineSplit(str(message))
		for i in range(len(lines)):
			if i > 0:
				msg_nosplit(lines[i])
			else:
				msg_nosplit(lines[i], title, reset)
			if (i > 1) and (wait > 0):
				time.sleep(wait)
	except BaseException as e:
		print("!!! Exception in easydraw.msg !!!")
		print(e)

def lineCentered(pos_y, line, font, color):
	if font:
		width = display.getTextWidth(line, font)
	else:
		width = display.getTextWidth(line)
	pos_x = int((display.width()-width) / 2)
	if font:
		display.drawText(pos_x, pos_y, line, color, font)
	else:
		display.drawText(pos_x, pos_y, line, color)

def messageCentered(message, firstLineTitle=True, png=None):
	#try:
	if 1:
		font1 = "Roboto_Regular18"
		font2 = "Roboto_Regular12"
		color = 0x000000
		display.drawFill(0xFFFFFF)
		parts = message.split("\n")
		lines = []
		font = font1
		for part in parts:
			if len(part) < 1:
				lines.append("")
			else:
				lines.extend(lineSplit(part, display.width(), font))
			if firstLineTitle:
				font = font2
		
		offset_y = int(display.height()/2) #Half of the screen height
		offset_y -= 9 #Height of the first line divided by 2
		if firstLineTitle:
			offset_y -= 6*len(lines)-1 #Height of font1 divided by 2
		else:
			offset_y -= 9*len(lines)-1 #Height of font2 divided by 2

		if png != None:
			try:
				img_info = badge.png_info(png)
				offset_y -= int(img_info[1] / 2) + 4
				img_x = int((display.width() - img_info[0]) / 2)
				display.drawPng(img_x, offset_y, png)
				offset_y += img_info[1] + 8 
			except:
				pass
		
		lineCentered(offset_y, lines[0], font1, color)
		offset_y += 18
		
		for i in range(len(lines)-1):
			if not firstLineTitle:
				lineCentered(offset_y, lines[i+1], font1, color)
				offset_y += 18
			else:
				lineCentered(offset_y, lines[i+1], font2, color)
				offset_y += 12
		display.flush()
	#except:
	#	print("!!! Exception in easydraw.messageCentered !!!")

def nickname(y = 5, font = "freesansbold12", color = 0x000000, unusedParameter=None):
	nick = machine.nvs_getstr("owner", "name")
	if not nick:
		return y
	lines = lineSplit(nick, display.width(), font)
	for i in range(len(lines)):
		line = lines[len(lines)-i-1]
		pos_x = int((display.width()-display.getTextWidth(line, font)) / 2)
		lineHeight = display.getTextHeight(line, font)
		if font:
			display.drawText(pos_x, y+lineHeight*(len(lines)-i-1), line, color, font)
		else:
			display.drawText(pos_x, y+lineHeight*(len(lines)-i-1), line, color)
	return len(lines) * lineHeight

def battery(on_usb, vBatt, charging):
	pass

def disp_string_right_bottom(y, s, font="freesans9"):
	l = display.getTextWidth(s, font)
	display.drawText(display.width()-l, display.height()-(y+1)*14, s, 0x000000, font)

def lineSplit(message, width=None, font=version.font_default):
	words = message.split(" ")
	lines = []
	line = ""
    
	if width==None:
		width=display.width()
    
	for word in words:
		wordLength = display.getTextWidth(word, font)
		lineLength = display.getTextWidth(line, font)
		if wordLength > width:
			lines.append(line)
			lines.append(word)
			line = ""
		elif lineLength+wordLength < width:
			if (line==""):
				line = word
			else:
				line += " "+word
		else:
			lines.append(line)
			line = word
	if len(line) > 0:
		lines.append(line)
	return lines
