import machine, badge, term, time

pin = machine.Pin(25)
rtc = machine.RTC()
rtc.wake_on_ext0(pin = pin, level = 0)

def start_sleeping(sleepTime=0):
	badge.off()
	term.header(True, "Going to sleep...")
	if (sleepTime >= 86400000): #One day
		sleepTime = 0
	if (sleepTime < 1):
		print("Sleeping until touchbutton is pressed...")
	else:
		print("Sleeping for "+str(sleepTime)+"ms...")
	time.sleep(0.1)
	machine.deepsleep(sleepTime)

def reboot():
    machine.deepsleep(1)
