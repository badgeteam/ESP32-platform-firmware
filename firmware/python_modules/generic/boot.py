import esp, machine, sys, system, os

esp.rtcmem_write(0,0)
esp.rtcmem_write(1,0)

#Application starting
app = esp.rtcmem_read_string()
if app:
	esp.rtcmem_write_string("")
else:
	if not machine.nvs_getint("system", 'factory'):
		app = "factory"
	else:
		app = machine.nvs_getstr("system", 'default_app')
		#if not app: #This generic set of modules has no default app
			#app = 'dashboard.home'

try:
	print("Starting app '%s'..." % app)
	system.__current_app__ = app
	if app:
		__import__(app)
except BaseException as e:
	print("Fatal exception in the running app!")
	sys.print_exception(e)
