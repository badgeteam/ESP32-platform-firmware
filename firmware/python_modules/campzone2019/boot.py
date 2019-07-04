import esp, machine, sys, system

system.clear_boot_magic()

rtc = machine.RTC()
if (rtc.timezone()==""):
	timezone = machine.nvs_getstr('sys_timezone')
	if not timezone:
		timezone = 'CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00'
	rtc.timezone(timezone)

app = esp.rtcmem_read_string()
if app:
	print("Found app name in RTC memory: starting %s..." % load_me)
	esp.rtcmem_write_string("")
else:
	app = machine.nvs_getstr('sys_home')
	if not app:
		app = 'dashboard'

try:
	system.__current_app__ = app
	__import__(app)
except BaseException as e:
	print("Fatal exception in the running app!")
	sys.print_exception(e)
