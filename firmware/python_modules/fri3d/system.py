import machine

def reboot():
	machine.deepsleep(2)

def sleep(duration=0, status=False):
	import time, os
	
	if (duration >= 86400000): #One day
		duration = 0
	if status:
		import term
		if duration < 1:
			term.header(True, "Sleeping forever...")
		else:
			term.header(True, "Sleeping for "+str(duration)+"ms...")
	time.sleep(0.05)
	machine.deepsleep(duration)

def start(app, status=False):
	if status:
		import term
		if app == "" or app == "launcher":
			term.header(True, "Loading menu...")
		else:
			term.header(True, "Loading application "+app+"...")
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
		import term
		term.header(True, "Starting update...")
	rtc = machine.RTC()
	rtc.write(0,1)
	rtc.write(1,254)
	reboot()

def serialWarning():
	pass
	
def crashedWarning():
	pass

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
