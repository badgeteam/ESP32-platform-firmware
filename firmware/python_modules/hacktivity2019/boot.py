import machine, sys, system, time, buttons, samd

rtc = machine.RTC()
rtc.write(0,0)
rtc.write(1,0)

__chk_recovery = False

if machine.nvs_getint("system", 'factory_checked'):
	__chk_recovery = machine.wake_reason() == (7, 0) and buttons.value(buttons.BTN_OK)

samd.backlight(255)

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
