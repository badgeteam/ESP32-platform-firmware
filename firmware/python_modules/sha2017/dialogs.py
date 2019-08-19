### Author: EMF Badge team
### Author: SHA2017Badge team
### Description: Some basic UGFX powered dialogs
### License: MIT

import machine, display, utime as time, buttons, ugfx, keyboard

wait_for_interrupt = True
button_pushed = ''

def notice(text, title="Notice", close_text="Close", width = None, height = None, font="Roboto_Regular12"):
	"""Show a notice which can be closed with button A.

	The caller is responsible for flushing the display after the user has confirmed the notice.
	"""
	prompt_boolean(text, title = title, true_text = close_text, false_text = None, width = width, height = height, font=font)

	
def _button(x, y, text, color, font):
	display.drawText(x, y, text, color, font)
	
__cb = None
def __asyncSuccess(evt):
	if evt and __cb:
		__cb(True)

def __asyncCancel(evt):
	if evt and __cb:
		__cb(False)

def prompt_boolean(text, title="Notice", true_text="Yes", false_text="No", width = None, height = None, font="Roboto_Regular12", cb=None):
	"""A simple one and two-options dialog

	if 'false_text' is set to None only one button is displayed.
	If both 'false_text' and 'true_text' are given a boolean is returned, press B for false, A for true.

	Pass along a 'cb' callback to make the dialog async, which is needed to make input work when used from a callback

	The caller is responsible for flushing the display after processing the response.
	"""
	global wait_for_interrupt, button_pushed, __cb
	
	__cb = cb

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
		    
	display.drawFill(0xFFFFFF)
	display.drawRect(0, 0, display.width(), 14, True, 0)
	display.drawText(0, 0, title, 0xFFFFFF, "Roboto_Regular12")

	if false_text:
		false_text = "B: " + false_text
	true_text = "A: " + true_text
	
	display.drawText(0, 36, ugfx.wordWrap(text, None, font), 0x000000, font)
	
	if false_text:
		_button(10, height-display.getTextHeight(false_text, font), false_text, 0x000000, font)
		_button((width - display.getTextWidth(true_text, font) - 10), height - display.getTextHeight(true_text, font), true_text, 0x000000, font)
	else:
		_button(width - 10 - display.getTextWidth(true_text, font), height - display.getTextHeight(true_text, font), true_text, 0x000000, font)

	display.flush()

	if cb:
		ugfx.input_attach(ugfx.BTN_A, __asyncSuccess)
		ugfx.input_attach(ugfx.BTN_B, __asyncCancel)
		#Done :-)
	else:
		ugfx.input_attach(buttons.BTN_A, None)
		ugfx.input_attach(buttons.BTN_B, None)
		
		while True:
			if buttons.value(buttons.BTN_A):
				display.drawFill(0xFFFFFF)
				display.flush()
				while buttons.value(buttons.BTN_A) or buttons.value(buttons.BTN_B):
					time.sleep(0.1)
				return True
			if buttons.value(buttons.BTN_B):
				display.drawFill(0xFFFFFF)
				display.flush()
				while buttons.value(buttons.BTN_A) or buttons.value(buttons.BTN_B):
					time.sleep(0.1)
				return False

def prompt_text(description, init_text = "", true_text="OK", false_text="Back", width = 300, height = 200, font="Roboto_BlackItalic24", cb=None):
	keyboard.show(description, init_text, cb)
