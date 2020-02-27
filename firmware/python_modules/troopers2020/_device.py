import os, machine, display, easydraw, time, neopixel

def configureWakeupSource():
	machine.RTC().wake_on_ext0(pin = machine.Pin(39), level = 0) # pca9555 interrupt
	return True

def prepareForSleep():
	try:
		os.umountsd()
	except:
		pass
	neopixel.send(bytes([0]*24)) # Turn off LEDs
	configureWakeupSource()

def prepareForWakeup():
	time.sleep(0.05) # Give the SD card time to initialize itself
	os.mountsd()

def showLoadingScreen(app=""):
	try:
		display.drawFill(0x000000)
		display.drawText( 0, 28, "LOADING APP...", 0xFFFFFF, "org18")
		display.drawText( 0, 52, app,              0xFFFFFF, "org18")
		display.flush()
	except:
		pass

def showMessage(message="", icon=None):
	easydraw.messageCentered(message, False, icon)

def setLedPower(state):
	pass
