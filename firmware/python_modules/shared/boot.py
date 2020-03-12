import machine, sys, system, time
import _device as device

rtc = machine.RTC()
rtc.write(0,0)
rtc.write(1,0)

device.prepareForWakeup()

__chk_recovery = False
fc_level = machine.nvs_getint("system", 'factory_checked') or 0
recovery_button = None

if fc_level >= 3:
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
		if fc_level < 3:
			app = "factory_checks"
		else:
			app = machine.nvs_getstr("system", 'default_app')
			if not app:
				app = 'dashboard.home'
        del fc_level

del __chk_recovery
del rtc
del recovery_button

if app and not app == "shell":
	try:
		print("Starting app '%s'..." % app)
		system.__current_app__ = app
		if app:
			__import__(app)
	except KeyboardInterrupt:
		system.launcher()
	except BaseException as e:
		sys.print_exception(e)
		if not machine.nvs_get_u8("system", "ignore_crash"):
			print("Fatal exception in the running app!")
			system.crashedWarning()
			time.sleep(3)
			system.launcher()
