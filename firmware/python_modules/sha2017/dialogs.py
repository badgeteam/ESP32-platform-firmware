### Author: EMF Badge team
### Author: SHA2017Badge team
### Description: Some basic UGFX powered dialogs
### License: MIT

import machine, display, utime as time, mpr121, ugfx

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
	print("Keyboard is not yet implemented...")
	return "NOT IMPLEMENTED"

def prompt_option(options, index=0, text = "Please select one of the following:", title=None, select_text="OK", none_text=None):
	"""Shows a dialog prompting for one of multiple options

	If none_text is specified the user can use the B or Menu button to skip the selection
	if title is specified a blue title will be displayed about the text
	"""
	global wait_for_interrupt, button_pushed

	ugfx.set_default_font("Roboto_Regular12")
	window = ugfx.Container(5, 5, ugfx.width() - 10, ugfx.height() - 10)
	window.show()

	list_y = 30
	if title:
		window.text(5, 10, title, ugfxBLACK)
		window.line(0, 25, ugfx.width() - 10, 25, ugfx.BLACK)
		window.text(5, 30, text, ugfx.BLACK)
		list_y = 50
	else:
		window.text(5, 10, text, ugfx.BLACK)

	options_list = ugfx.List(5, list_y, ugfx.width() - 25, 180 - list_y, parent = window)

	for option in options:
		if isinstance(option, dict) and option["title"]:
			options_list.add_item(option["title"])
		else:
			options_list.add_item(str(option))
	options_list.selected_index(index)

	select_text = "A: " + select_text
	if none_text:
		none_text = "B: " + none_text

	button_select = ugfx.Button(5, ugfx.height() - 50, 140 if none_text else ugfx.width() - 25, 30 , select_text, parent=window)
	button_none = ugfx.Button(ugfx.width() - 160, ugfx.height() - 50, 140, 30 , none_text, parent=window) if none_text else None

	try:
		ugfx.input_init()

		wait_for_interrupt = True
		while wait_for_interrupt:
			if button_pushed == "A": return options[options_list.selected_index()]
			if button_pushed == "B": return None
			if button_none and button_pushed == "START": return None
			time.sleep(0.2)

	finally:
		window.hide()
		window.destroy()
		options_list.destroy()
		button_select.destroy()
		if button_none: button_none.destroy()
		ugfx.poll()


def pressed_a(pushed):
	global wait_for_interrupt, button_pushed
	wait_for_interrupt = False
	button_pushed = 'A'

def pressed_b(pushed):
	global wait_for_interrupt, button_pushed
	wait_for_interrupt = False
	button_pushed = 'B'

def pressed_start(pushed):
	global wait_for_interrupt, button_pushed
	wait_for_interrupt = False
	button_pushed = 'START'

ugfx.input_attach(ugfx.BTN_A, pressed_a)
ugfx.input_attach(ugfx.BTN_B, pressed_b)
ugfx.input_attach(ugfx.BTN_START, pressed_start)

class WaitingMessage:
	"""Shows a dialog with a certain message that can not be dismissed by the user"""
	def __init__(self, text = "Please Wait...", title="SHA2017Badge"):
		self.window = ugfx.Container(30, 30, ugfx.width() - 60, ugfx.height() - 60)
		self.window.show()
		self.window.text(5, 10, title, ugfx.BLACK)
		self.window.line(0, 30, ugfx.width() - 60, 30, ugfx.BLACK)
		self.label = ugfx.Label(5, 40, self.window.width() - 10, ugfx.height() - 40, text = text, parent=self.window)

		# Indicator to show something is going on
		self.indicator = ugfx.Label(ugfx.width() - 100, 0, 20, 20, text = "...", parent=self.window)
		self.timer = machine.Timer(-1)
		self.timer.init(period=2000, mode=self.timer.PERIODIC, callback=lambda t:self.indicator.visible(not self.indicator.visible()))

	def destroy(self):
		self.timer.deinit()
		self.label.destroy()
		self.indicator.destroy()
		self.window.destroy()

	def text(self):
		return self.label.text()

	def text(self, value):
		self.label.text(value)

	def __enter__(self):
		return self

	def __exit__(self, exc_type, exc_value, traceback):
		self.destroy()
