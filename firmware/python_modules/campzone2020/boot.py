import machine, sys, system, time, display, touchbuttons

rtc = machine.RTC()
rtc.write(0,0)
rtc.write(1,0)

def on_touch(state):
	if state & 512:
		system.home()
touchbuttons.set_handler(on_touch)

__chk_recovery = False

if machine.nvs_getint("system", 'factory_checked'):
	try:
		__chk_recovery = machine.wake_reason() == (7, 0)
	except:
		pass

#Application starting
if machine.wake_reason() == (0, 0):
	# Boot splash screen
	app = "bootsplash"
elif machine.wake_reason() == (7, 0):
	# Recovery mode
	app = "dashboard.recovery"
elif not machine.nvs_getint("system", 'factory_checked'):
	# Factory check mode
	app = "factory_checks"
else:
	app = rtc.read_string()
	if not app:
		if not machine.nvs_getint("system", 'factory_checked') == 2:
			app = "factory_checks"
		else:
			app = machine.nvs_getstr("system", 'default_app')
			if not app:
				app = 'dashboard.home'

try:
	import os
	os.mountsd()
	del os
except:
	print('Could not mount SD card')
	pass

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
