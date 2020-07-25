import ugfx, badge, sys, uos as os, system, consts, easydraw, virtualtimers, tasks.powermanagement as pm, dialogs, time, ujson, sys, display

# Application list

apps = []

def add_app(app,information):
	global apps
	try:
		title = information["name"]
	except:
		title = app
	try:
		category = information["category"]
	except:
		category = ""
	info = {"file":app,"title":title,"category":category}
	apps.append(info)

def populate_apps():
	global apps
	apps = []
	try:
		userApps = os.listdir('lib')
	except OSError:
		userApps = []
	add_app("dashboard.tools.reset_default",{"name":"Reset default app", "category":"system"})
	add_app("dashboard.tools.erase_storage",{"name":"Erase storage", "category":"system"})
	add_app("dashboard.launcher",{"name":"Launcher", "category":"system"})
	add_app("dashboard.tools.factory_reset",{"name":"Factory reset", "category":"system"})
	add_app("dashboard.tools.update_firmware",{"name":"Firmware update", "category":"system"})
	add_app("dashboard.settings.wifi",{"name":"Configure WiFi", "category":"system"})

# List as shown on screen
currentListTitles = []
currentListTargets = []

def populate_category(category="",system=True):
	global apps
	global currentListTitles
	global currentListTargets
	currentListTitles = []
	currentListTargets = []
	for app in apps:
		if (category=="" or category==app["category"] or (system and app["category"]=="system")) and (not app["category"]=="hidden"):
			currentListTitles.append(app["title"])
			currentListTargets.append(app)
			
def populate_options():
	global options
	options = ugfx.List(0,0,int(ugfx.width()/2),ugfx.height())
	global currentListTitles
	for title in currentListTitles:
		options.add_item(title)
	
# Run app
		
def run():
	global options
	selected = options.selected_index()
	options.destroy()
	global currentListTargets
	system.start(currentListTargets[selected]["file"], True)

# Path

def expandhome(s):
	if "~/" in s:
		h = os.getenv("HOME")
		s = s.replace("~/", h + "/")
	return s

def get_install_path():
	global install_path
	if install_path is None:
		# sys.path[0] is current module's path
		install_path = sys.path[1]
	install_path = expandhome(install_path)
	return install_path


# Actions        
def input_a(pressed):
	pm.feed()
	if pressed:
		run()
	
def input_b(pressed):
	pm.feed()
	if pressed:
		system.home()

def input_select(pressed):
	pm.feed()
	
def input_other(pressed):
	pm.feed()
	if pressed:
		display.flush(display.FLAG_LUT_FASTEST)

# Power management
def pm_cb(dummy):
	system.home()

def init_power_management():
	virtualtimers.activate(1000) # Start scheduler with 1 second ticks
	pm.set_timeout(5*60*1000) # Set timeout to 5 minutes
	pm.callback(pm_cb) # Go to splash instead of sleep
	pm.feed() # Feed the power management task, starts the countdown...

# Main application
def start():
	ugfx.set_lut(ugfx.LUT_FASTER)
	ugfx.clear(ugfx.WHITE)
	x0 = int(display.width()/2)
	display.drawText(x0+10, 0, "Recovery mode!", 0x000000, "roboto_regular12")

	global options
	global install_path
	options = None
	install_path = None

	ugfx.input_attach(ugfx.BTN_A, input_a)
	ugfx.input_attach(ugfx.BTN_B, input_b)
	ugfx.input_attach(ugfx.BTN_SELECT, input_select)
	ugfx.input_attach(ugfx.JOY_UP, input_other)
	ugfx.input_attach(ugfx.JOY_DOWN, input_other)
	ugfx.input_attach(ugfx.JOY_LEFT, input_other)
	ugfx.input_attach(ugfx.JOY_RIGHT, input_other)
	ugfx.input_attach(ugfx.BTN_START, input_other)

	populate_apps()
	populate_category()
	populate_options()

	# do a greyscale flush on start
	ugfx.flush(ugfx.GREYSCALE)

start()
init_power_management()

def goToSleep(unused_variable=None):
	system.home()

import term_menu
umenu = term_menu.UartMenu(goToSleep, pm, badge.safe_mode(), "< Back")
umenu.main()
