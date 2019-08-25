import machine, sys, system, time

rtc = machine.RTC()
rtc.write(0,0)
rtc.write(1,0)

try:
	import buttons
except:
	pass

def __chk_recovery():
	try:
		try:
			recovery_button = buttons.BTN_START
		except:
			# This badge does not have a START button, use the B button instead
			recovery_button = buttons.BTN_B
		return machine.wake_reason() == (7, 0) and buttons.value(recovery_button)
	except:
		return False

#Application starting
if __chk_recovery():
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


# If present, include the service for undervoltage detection
try:
	__import__('tasks/undervoltagemon')
except:
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

if app and app == "shell":
	print("\nWelcome to the python shell of your badge!\nCheck out https://wiki.badge.team/ for instructions.")
