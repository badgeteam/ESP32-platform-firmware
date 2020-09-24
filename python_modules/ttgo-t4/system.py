import machine

def reboot():
	machine.deepsleep(2)

def sleep(duration=0, status=False):
	import time, os, badge
	#machine.RTC().wake_on_ext0(pin = machine.Pin(34), level = 0) # MPR121 interrupt
	#machine.RTC().wake_on_ext1([machine.Pin(5, machine.Pin.IN, machine.Pin.PULL_UP)], 0)
	if (duration >= 86400000): #One day
		duration = 0
	if status:
		import term
		if duration < 1:
			term.header(True, "Sleeping until a touch button is pressed!")
		else:
			term.header(True, "Sleeping for "+str(duration)+"ms...")
	time.sleep(0.05)
	machine.deepsleep(duration)

def start(app, status=False):
	if status:
		import term, easydraw, display
		if app == "" or app == "launcher":
			term.header(True, "Loading menu...")
			#easydraw.messageCentered("Loading the menu...", False, "/media/busy.png")
		else:
			term.header(True, "Loading application "+app+"...")
			#easydraw.messageCentered("Loading '"+app+"'...", False, "/media/busy.png")
		try:
			info = display.pngInfo("/media/busy.png")
			display.drawPng((display.width()-info[0])//2, (display.height()-info[1])//2, "/media/busy.png")
		except:
			easydraw.messageCentered("Loading...", False, "/media/busy.png")
	machine.RTC().write_string(app)
	reboot()

def home(status=False):
	start("", status)

def launcher(status=False):
	start("dashboard.launcher", status)

def shell(status=False):
	start("shell", status)

# Over-the-air updating

def ota(status=False):
	if status:
		import term, easydraw
		term.header(True, "Starting update...")
		easydraw.messageCentered("Starting update...", False, "/media/busy.png")
	rtc = machine.RTC()
	rtc.write(0,1)
	rtc.write(1,254)
	reboot()

def serialWarning():
	import easydraw
	easydraw.messageCentered("This app can only be controlled using the USB-serial connection!", False, "/media/crown.png")
	
def crashedWarning():
	import easydraw
	easydraw.messageCentered("FATAL ERROR\nthe app has crashed", False, "/media/alert.png")

def isColdBoot():
	if machine.wake_reason() == (7, 0):
		return True
	return False

def isWakeup(fromTimer=True,fromButton=True, fromIr=True, fromUlp=True):
	if fromButton and machine.wake_reason() == (3, 1):
		return True
	if fromIr     and machine.wake_reason() == (3, 2):
		return True
	if fromTimer  and machine.wake_reason() == (3, 4):
		return True
	if fromUlp    and machine.wake_reason() == (3, 5):
		return True
	return False

__current_app__ = None

def currentApp():
	return __current_app__
