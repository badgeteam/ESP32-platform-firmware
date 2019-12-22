import os, machine, mpr121, display, mascot, easydraw


def configureWakeupSource():
	return False

def prepareForSleep():
	return False

def prepareForWakeup():
	return False

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
