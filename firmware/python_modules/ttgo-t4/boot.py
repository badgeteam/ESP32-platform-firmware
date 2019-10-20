import machine, sys, system, time, os

# Fix SD card
sd_d3 = machine.Pin(13, machine.Pin.OUT)
sd_d3.value(1)


rtc = machine.RTC()
rtc.write(0,0)
rtc.write(1,0)

sdPower = machine.Pin(19, machine.Pin.OUT)
sdPower.value(True)
time.sleep(0.05)
os.mountsd()

__chk_recovery = False

if machine.nvs_getint("system", 'factory_checked'):
	try:
		import buttons
		try:
			#Use the START button if available
			recovery_button = buttons.BTN_START
		except:
			#Else use the B button
			recovery_button = buttons.BTN_B
		__chk_recovery = machine.wake_reason() == (7, 0) and buttons.value(recovery_button)
	except:
		pass


#Application starting
if __chk_recovery:
	app = "dashboard.recovery"
else:
	app = rtc.read_string()
	if not app:
		if not machine.nvs_getint("system", 'factory_checked') == 2:
			app = "factory_checks"
		else:
			app = machine.nvs_getstr("system", 'default_app')
			if not app:
				app = 'dashboard.home'

if app and not app == "shell":
	try:
		print("Starting app '%s'..." % app)
		system.__current_app__ = app
		if app:
			__import__(app)
	except BaseException as e:
		sys.print_exception(e)
		if not machine.nvs_get_u8("system", "ignore_crash"):
			print("Fatal exception in the running app!")
			system.crashedWarning()
			time.sleep(3)
			system.launcher()

if app and app == "shell":
	print("\nWelcome to the python shell of your badge!\nCheck out https://wiki.badge.team/ for instructions.")
