### Author: EMF Badge team
### Author: SHA2017Badge team
### Description: Some basic UGFX powered dialogs
### License: MIT

import machine, display, utime as time, mpr121, ugfx, keyboard

wait_for_interrupt = True
button_pushed = ''

def notice(text, title="Notice", close_text="Close", width = None, height = None, font="Roboto_Regular12"):
	"""Show a notice which can be closed with button A.

	The caller is responsible for flushing the display after the user has confirmed the notice.
	"""
	prompt_boolean(text, title = title, true_text = close_text, false_text = None, width = width, height = height, font=font)

def _label(window, x, y, text, color, font):
	display.drawText(window, x, y, text, color, font)
	
def _button(window, x, y, text, color, font):
	display.drawText(window, x, y, text, color, font)
	

def prompt_boolean(text, title="Notice", true_text="Yes", false_text="No", width = None, height = None, font="Roboto_Regular12", cb=None):
	"""A simple one and two-options dialog

	if 'false_text' is set to None only one button is displayed.
	If both 'false_text' and 'true_text' are given a boolean is returned, press B for false, A for true.

	Pass along a 'cb' callback to make the dialog async, which is needed to make input work when used from a callback

	The caller is responsible for flushing the display after processing the response.
	"""
	global wait_for_interrupt, button_pushed

	if width == None:
		width = display.width()
	if height == None:
		height = display.height()

	x = (display.width() - width) // 2
	y = (display.height() - height) // 2
	if (x < 0):
		x = 0
	if (y < 0):
		y = 0
	#print("Container", x, y, width, height)
	try:
		display.windowCreate("dialog", width, height)
	except:
		pass
	display.drawFill("dialog", 0xFFFFFF)
	display.windowShow("dialog")
	display.drawText("dialog", 5, 10, title, 0x000000, font)
	display.drawLine("dialog", 0, 30, width, 30, 0x000000)

	if false_text:
		false_text = "B: " + false_text
		true_text = "A: " + true_text

	def done(result):
		display.windowRemove("dialog")
		if cb:
			cb(result)
		return result

	def syncSuccess(evt):
		if evt:
			# We'd like promises here, but for now this should do
			global wait_for_interrupt, button_pushed
			button_pushed = "A"
			wait_for_interrupt = False
	def syncCancel(evt):
		if evt:
			# We'd like promises here, but for now this should do
			global wait_for_interrupt, button_pushed
			button_pushed = "B"
			wait_for_interrupt = False

	def asyncSuccess(evt):
		if evt:
			done(True)
	def asyncCancel(evt):
		if evt:
			done(False)

	_label("dialog", 5, 35, text, 0x000000, font)
	if false_text:
		_button("dialog", 10, height-display.getTextHeight(false_text, font), false_text, 0x000000, font)
		_button("dialog", (width - display.getTextWidth(true_text, font) - 10), height - display.getTextHeight(true_text, font), true_text, 0x000000, font)
	else:
		_button("dialog", width - 10 - display.getTextWidth(true_text, font), height - display.getTextHeight(true_text, font), true_text, 0x000000, font)

	#label = ugfx.Label(5, 30, width - 10, height - 80, text = text, parent=window)
	#button_no = ugfx.Button(5, height - 40, width // 2 - 15, 30, false_text, parent=window) if false_text else None
	#button_yes = ugfx.Button(width // 2 + 5 if true_text else 5, height - 40, width // 2 - 15 if false_text else width - 10, 30, true_text, parent=window)
	#button_yes.set_focus()

	display.flush()

	if false_text: mpr121.attach(1, asyncCancel if cb else syncCancel)
	mpr121.attach(0, asyncSuccess if cb else syncSuccess)

	if cb:
		return
	else:
		wait_for_interrupt = True
		while wait_for_interrupt:
			time.sleep(0.2)

		if button_pushed == "B": return done(False)
		return done(True)

def prompt_text(description, init_text = "", true_text="OK", false_text="Back", width = 300, height = 200, font="Roboto_BlackItalic24", cb=None):
	keyboard.show(description, init_text, cb)
