import os, machine, display, mascot, easydraw, time

PIN_SDCARD = 19
sdcardPower = machine.Pin(PIN_SDCARD, machine.Pin.OUT)

def configureWakeupSource():
	machine.RTC().wake_on_ext0(pin = machine.Pin(25), level = 0)
	machine.RTC().wake_on_ext1([
		machine.Pin(26, machine.Pin.IN, machine.Pin.PULL_UP)
	], 0)
	return True

def prepareForSleep():
	try:
		os.umountsd()
	except:
		pass
	configureWakeupSource()

def prepareForWakeup():
	pass

def showLoadingScreen(app=""):
	try:
		display.drawFill(0x000000)
		display.drawPng( 64,  0, mascot.snek                        )
		display.drawText( 0, 28, "LOADING APP...", 0xFFFFFF, "org18")
		display.drawText( 0, 52, app,              0xFFFFFF, "org18")
		display.flush()
	except:
		pass

def showMessage(message="", icon=None):
	easydraw.messageCentered(message, False, icon)

def setLedPower(state):
	pass
