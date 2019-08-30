import display, time, virtualtimers, buzzer, buttons

def drawEye(offset, color):
	display.drawPixel(3+offset,0,color)
	display.drawPixel(2+offset,1,color)
	display.drawPixel(4+offset,1,color)
	display.drawPixel(2+offset,2,color)
	display.drawPixel(4+offset,2,color)
	display.drawPixel(2+offset,3,color)
	display.drawPixel(4+offset,3,color)
	display.drawPixel(3+offset,4,color)

def drawEyeClosed(offset, color):
	display.drawPixel(2+offset,2,color)
	display.drawPixel(3+offset,2,color)
	display.drawPixel(4+offset,2,color)

# Animation :-)
animation_step = 0
animation_type = 0
def animation_task():
	global animation_step, animation_type
	if animation_type == 0:
		if animation_step == 0:
			display.drawFill()
			drawEye(0,0xFFFFFF)
			drawEye(7,0xFFFFFF)
			display.flush()
			animation_step += 1
		elif animation_step < 10:
			animation_step+=1
		else:
			display.drawFill()
			drawEye(0,0xFFFFFF)
			drawEyeClosed(7,0xFFFFFF)
			display.flush()
			animation_step = 0
	elif animation_type == 1:
		display.drawFill(animation_step | (animation_step<<8) | (animation_step<<16))
		animation_step += 1
		if animation_step > 0xFF:
			animation_step = 0
	elif animation_type == 2:
		if animation_step == 0:
			display.drawFill()
			drawEye(0,0xFFFFFF)
			drawEye(7,0xFFFFFF)
			display.flush()
			animation_step += 1
		elif animation_step < 10:
			animation_step+=1
		else:
			display.drawFill()
			drawEyeClosed(0,0xFFFFFF)
			drawEyeClosed(7,0xFFFFFF)
			display.flush()
			animation_step = 0
	elif animation_type == 3:
		for x in range(14):
			for y in range(5):
				color = x + (y*14)
				display.drawPixel(x, y, color | (color<<8) | (color<<16))
	return 100

virtualtimers.activate(100)
virtualtimers.new(1, animation_task)

def btnA(pressed):
	global animation_type
	if pressed:
		animation_type = 1
	elif animation_type == 1:
		animation_type = 0

def btnB(pressed):
	global animation_type
	if pressed:
		animation_type = 2
	elif animation_type == 2:
		animation_type = 0

def btnC(pressed):
	global animation_type
	if pressed:
		animation_type = 3
	elif animation_type == 3:
		animation_type = 0

buttons.attach(buttons.BTN_0, btnB)
buttons.attach(buttons.BTN_1, btnA)
buttons.attach(buttons.BTN_BOOT, btnC)

song = "20thCenFox:d=16,o=5,b=140:b,8p,b,b,2b,p,c6,32p,b,32p,c6,32p,b,32p,c6,32p,b,8p,b,b,b,32p,b,32p,b,32p,b,32p,b,32p,b,32p,b,32p,g#,32p,a,32p,b,8p,b,b,2b,4p,8e,8g#,8b,1c#6,8f#,8a,8c#6,1e6,8a,8c#6,8e6,1e6,8b,8g#,8a,2b"

buzzer.play(song)
