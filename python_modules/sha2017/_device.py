# Device specific system functions: SHA2017
import mpr121, eink, mascot, easydraw, display, time, os, machine

def configureWakeupSource():
	machine.RTC().wake_on_ext0(pin = machine.Pin(25), level = 0)
	machine.RTC().wake_on_ext1([machine.Pin(12, machine.Pin.IN, machine.Pin.PULL_UP)], 0)
	return True

def prepareForSleep():
	configureWakeupSource()
	try:
		os.umountsd()
	except:
		pass
	mpr121.set(10, False)
	eink.busy_wait()

def prepareForWakeup():
	mpr121.set(10, True)
	time.sleep(0.05) # Give the SD card time to initialize itself
	os.mountsd()

def showLoadingScreen(app=""):
	hourglass = b"\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x005\x00\x00\x00Z\x01\x00\x00\x00\x000?#R\x00\x00\x00jIDAT(\xcf\xad\xd2A\x12\x80 \x08\x05Po\xc0\x95\xbd\xc1\xbf.\xbb\x1f\x91\x8d5\x90:\x15.\xde\x02\xc6\x01\xb4\x94Y\xd0B\x07NB\xbdN\x7f\x10@\xea\x99W\xcdmy!XS\xb9\xcbje2P\x05\x95\xa9t\xed\x12\xf9$\xec\\\xee\x0bZ\x1f\xf4~\xd0\xfa\xba\x0b\x9b\x03}\x9e`\xdf'\x1e\\\xdb\xe3\xea{\x1c\x11m\xf9\xd7\xffm\xfc\xef6\x03^\xc2\x1c\x0eD=\xf5\x00\x00\x00\x00IEND\xaeB`\x82"
	try:
		display.drawFill(0xFFFFFF)
		text = "Loading...\n\n{}".format(app)
		display.drawText(91, (display.height()-display.getTextHeight(text, "ocra16"))//2, text, 0x000000, "ocra16")
		display.drawPng(19,19,hourglass)
		display.flush(display.FLAG_LUT_FASTEST)
	except:
		pass

def showMessage(message="", icon=None):
	easydraw.messageCentered(message, False, icon)
