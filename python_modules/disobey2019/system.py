import machine, time, term, easydraw, display, samd

def reboot():
	for i in range(6):
		samd.led(i,0,0,0)
	machine.deepsleep(2)

def sleep(duration=0, status=False):
	for i in range(6):
		samd.led(i,0,0,0)
	machine.RTC().wake_on_ext0(pin = machine.Pin(25), level = 0)
	if (duration >= 86400000): #One day
		duration = 0
	if status:
		if duration < 1:
			term.header(True, "Sleeping until touch button is pressed!")
		else:
			term.header(True, "Sleeping for "+str(duration)+"ms...")
	time.sleep(0.05)
	machine.deepsleep(duration)

def start(app, status=False):
	for i in range(6):
		samd.led(i,0,0,0)
	if status:
		appName = app
		if app == "" or app.startswith("dashboard."):
			term.header(True, "Loading...")
			appName = ""
		else:
			term.header(True, "Loading application "+app+"...")
		try:
			display.drawFill()
			import mascot
			display.drawPng( 64,  0, mascot.snek                 )
			display.drawText( 0, 28, "LOADING APP...", 0, "org18")
			display.drawText( 0, 52, appName,          0, "org18")
			display.flush()
			time.sleep_ms(10) #Give display some time
		except:
			easydraw.messageCentered("Loading...", False, "/media/busy.png")
	machine.RTC().write_string(app)
	reboot()

def home(status=True):
	start("", status)

def launcher(status=True):
	start("dashboard.launcher", status)

def shell(status=True):
	start("shell", status)

# Over-the-air updating

def ota(status=True):
	if status:
		term.header(True, "Starting update...")
		easydraw.messageCentered("Starting update...", False, "/media/busy.png")
	rtc = machine.RTC()
	rtc.write(0,1)
	rtc.write(1,254)
	reboot()

def serialWarning():
	easydraw.messageCentered("This app can only be controlled using the USB-serial connection!", False, "/media/crown.png")
	
def crashedWarning():
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
