import ugfx, badge, sys, uos as os, system, consts, easydraw, virtualtimers, tasks.powermanagement as pm, dialogs, time, ujson, sys, orientation, display

orientation.default()

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
	for app in userApps:
		add_app(app,read_metadata(app))
	add_app("dashboard.installer",{"name":"Installer", "category":"system"})
	add_app("dashboard.settings.nickname",{"name":"Set nickname", "category":"system"})
	add_app("dashboard.settings.wifi",{"name":"Configure WiFi", "category":"system"})
	add_app("dashboard.tools.update_apps",{"name":"Update apps", "category":"system"})
	add_app("dashboard.tools.update_firmware",{"name":"Firmware update", "category":"system"})
	add_app("dashboard.other.about",{"name":"About", "category":"system"})

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
	if orientation.isLandscape():
		options = ugfx.List(0,0,int(ugfx.width()/2),ugfx.height())
	else:
		options = ugfx.List(0,0,ugfx.width(),int(ugfx.height()-18*4))
	global currentListTitles
	for title in currentListTitles:
		options.add_item(title)

# Read app metadata
def read_metadata(app):
	try:
		install_path = get_install_path()
		info_file = "%s/%s/metadata.json" % (install_path, app)
		print("Reading "+info_file+"...")
		with open(info_file) as f:
			information = f.read()
		return ujson.loads(information)
	except BaseException as e:
		print("[ERROR] Can not read metadata for app "+app)
		sys.print_exception(e)
		information = {"name":app,"description":"","category":"", "author":"","revision":0}
		return [app,""]
	
# Uninstaller

def uninstall():
	global options
	selected = options.selected_index()
	options.destroy()
	
	global currentListTitles
	global currentListTargets
		
	if currentListTargets[selected]["category"] == "system":
		#dialogs.notice("System apps can not be removed!","Can not uninstall '"+currentListTitles[selected]+"'")
		easydraw.msg("System apps can not be removed!","Error",True)
		time.sleep(2)
		start()
		return
	
	def perform_uninstall(ok):
		global install_path
		if ok:
			easydraw.msg("Removing "+currentListTitles[selected]+"...", "Uninstalling...",True)
			install_path = get_install_path()
			for rm_file in os.listdir("%s/%s" % (install_path, currentListTargets[selected]["file"])):
				easydraw.msg("Deleting '"+rm_file+"'...")
				os.remove("%s/%s/%s" % (install_path, currentListTargets[selected]["file"], rm_file))
			easydraw.msg("Deleting folder...")
			os.rmdir("%s/%s" % (install_path, currentListTargets[selected]["file"]))
			easydraw.msg("Uninstall completed!")
		start()

	uninstall = dialogs.prompt_boolean('Are you sure you want to remove %s?' % currentListTitles[selected], cb=perform_uninstall)

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
	if pressed:
		uninstall()
	
def input_other(pressed):
	pm.feed()
	if pressed:
		ugfx.flush()

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
	ugfx.input_init()
	ugfx.set_lut(ugfx.LUT_FASTER)
	ugfx.clear(ugfx.WHITE)

	# Instructions
	if orientation.isLandscape():
		x0 = int(display.width()/2)
		display.font("fairlight8")
		currentY = 20
		display.cursor(x0+int((display.width()-x0)/2)-int(display.get_string_width("BADGE.TEAM")/2),currentY)
		display.print("BADGE.TEAM\n")
		display.font("pixelade9")
		(_, currentY) = display.cursor()
		display.cursor(x0+int((display.width()-x0)/2)-int(display.get_string_width("ESP32 platform")/2),currentY)
		display.print("ESP32 platform\n")
		display.line(x0,0,x0,display.height()-1,0x000000)
		display.textColor(0x000000)
		display.font("pixelade9")
		currentY = display.get_string_height(" ")*5-5
		display.line(x0,currentY,display.width()-1,currentY,0x000000)
		display.cursor(x0+5,currentY+5)
		display.print("A: Run\n")
		display.print("B: Return to home\n")
		display.print("SELECT: Uninstall app\n")
		(_, currentY) = display.cursor()
		display.line(x0,currentY,display.width()-1,currentY,0x000000)
		_ = display.cursor(x0+5,currentY+5)
		display.print(consts.INFO_FIRMWARE_NAME)
	else:
		ugfx.line(0, ugfx.height()-18*4, ugfx.width(), ugfx.height()-18*4, ugfx.BLACK)
		ugfx.string_box(0,ugfx.height()-18*4,ugfx.width(),18, " A: Run", "Roboto_Regular12", ugfx.BLACK, ugfx.justifyLeft)
		ugfx.string_box(0,ugfx.height()-18*3,ugfx.width(),18, " B: Return to home", "Roboto_Regular12", ugfx.BLACK, ugfx.justifyLeft)
		ugfx.string_box(0,ugfx.height()-18*2,ugfx.width(),18, " SELECT: Uninstall", "Roboto_Regular12", ugfx.BLACK, ugfx.justifyLeft)
		ugfx.line(0, ugfx.height()-18*1, ugfx.width(), ugfx.height()-18*1, ugfx.BLACK)
		ugfx.string_box(0,ugfx.height()-18*1,ugfx.width(),18, " " + consts.INFO_FIRMWARE_NAME, "Roboto_Regular12", ugfx.BLACK, ugfx.justifyLeft)

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
