import os, machine, mpr121, display, mascot, easydraw, time

PIN_SDCARD = 19
sdcardPower = machine.Pin(PIN_SDCARD, machine.Pin.OUT)

def configureWakeupSource():
	machine.RTC().wake_on_ext0(pin = machine.Pin(34), level = 0) #MPR121 touch controller interrupt
	#machine.RTC().wake_on_ext1([machine.Pin(5, machine.Pin.IN, machine.Pin.PULL_UP)], 0)
	return True

def prepareForSleep():
	try:
		os.umountsd()
	except:
		pass
	sdcardPower.value(False)
	configureWakeupSource()

def prepareForWakeup():
	sdcardPower.value(True)
	time.sleep(0.05) # Give the SD card time to initialize itself
	os.mountsd()

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
	mpr121.set(10, state)
