import ugfx, badge, sys, uos as os, system, consts, easydraw, virtualtimers, tasks.powermanagement as pm, dialogs, time, ujson, sys, orientation, display, machine, term, term_menu

orientation.default()

term.header(True, "Loading...")

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
	try:
		userApps = os.listdir('apps')
	except OSError:
		userApps = []
	for app in userApps:
		add_app(app,read_metadata(app))
	try:
		userApps = os.listdir('/sd/lib')
	except OSError:
		userApps = []
	for app in userApps:
		add_app(app,read_metadata(app))
	try:
		userApps = os.listdir('/sd/apps')
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
		if display.width() > 128:
			options = ugfx.List(0,0,int(ugfx.width()/2),ugfx.height())
		else:
			options = ugfx.List(0,0,ugfx.width(),ugfx.height())
	else:
		options = ugfx.List(0,0,ugfx.width(),int(ugfx.height()-18*4))
	global currentListTitles
	for title in currentListTitles:
		options.add_item(title)

# Read app metadata
def read_metadata(app):
	try:
		install_path = "/lib" #FIXME
		info_file = "%s/%s/metadata.json" % (install_path, app)
		#print("Reading "+info_file+"...")
		with open(info_file) as f:
			information = f.read()
		return ujson.loads(information)
	except BaseException as e:
		#print("[ERROR] Can not read metadata for app "+app)
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
		#print("System apps can not be removed.")
		dialogs.notice("Can not uninstall '"+currentListTitles[selected]+"'!\nSystem apps can not be removed!","UNINSTALL")
		#easydraw.msg("System apps can not be removed!","Error",True)
		#time.sleep(2)
		#print("Returning to menu.")
		start()
		return
	
	def perform_uninstall(ok):
		global install_path
		if ok:
			easydraw.msg("Removing "+currentListTitles[selected]+"...", "Uninstalling...",True)
			install_path = "/lib" #FIXME
			for rm_file in os.listdir("%s/%s" % (install_path, currentListTargets[selected]["file"])):
				easydraw.msg("Deleting '"+rm_file+"'...")
				os.remove("%s/%s/%s" % (install_path, currentListTargets[selected]["file"], rm_file))
			easydraw.msg("Deleting folder...")
			os.rmdir("%s/%s" % (install_path, currentListTargets[selected]["file"]))
			easydraw.msg("Uninstall completed!")
		start()

	uninstall = dialogs.prompt_boolean('Are you sure you want to remove %s?' % currentListTitles[selected], cb=perform_uninstall, title="UNINSTALL")

# Run app
		
def run():
	global options
	selected = options.selected_index()
	options.destroy()
	global currentListTargets
	system.start(currentListTargets[selected]["file"], True)

# Actions        
def input_a(pressed):
	pm.feed()
	if pressed:
		run()
	
def input_b(pressed):
	pm.feed()
	#if pressed:
	#	system.home()

def input_start(pressed):
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
		global einkNeedsUpdate
		einkNeedsUpdate = True
		#display.flush(display.FLAG_LUT_FASTEST)

# Power management
def pm_cb(dummy):
	system.home()

einkNeedsUpdate = False
def updateEink():
	global einkNeedsUpdate
	if einkNeedsUpdate:
		einkNeedsUpdate = False
		display.flush(display.FLAG_LUT_FASTEST)
	return 100

def init_power_management():
	pm.set_timeout(5*60*1000) # Set timeout to 5 minutes
	pm.callback(pm_cb) # Go to splash instead of sleep
	pm.feed(True)

# Main application
def start():
	ugfx.input_init()
	ugfx.set_lut(ugfx.LUT_FASTER)
	ugfx.clear(ugfx.WHITE)

	# Instructions
	if orientation.isLandscape():
		if display.width() > 128:
			x0 = int(display.width()/2)
			currentY = 20
			
			display.drawText(x0+((display.width()-x0)//2)-(display.getTextWidth("BADGE.TEAM", "fairlight12")//2), currentY, "BADGE.TEAM\n", 0x000000, "fairlight12")
			currentY += display.getTextHeight("BADGE.TEAM", "fairlight12")
			
			display.drawText(x0+int((display.width()-x0)/2)-int(display.getTextWidth("ESP32 platform", "roboto_regular12")/2), currentY, "ESP32 platform\n", 0x000000, "roboto_regular12")
			display.drawLine(x0,0,x0,display.height()-1,0x000000)
			pixHeight = display.getTextHeight(" ", "roboto_regular12")
			currentY = pixHeight*5
			
			lineY = display.height()-pixHeight*6-pixHeight//2
			display.drawLine(x0, lineY, display.width()-1, lineY, 0x000000)
			
			display.drawText(x0+5, display.height()-pixHeight*6, "A: Run\n", 0x000000, "roboto_regular12")
			display.drawText(x0+5, display.height()-pixHeight*5, "START: Return to home\n", 0x000000, "roboto_regular12")
			display.drawText(x0+5, display.height()-pixHeight*4, "SELECT: Uninstall app\n", 0x000000, "roboto_regular12")
			
			lineY = display.height()-pixHeight*2-pixHeight//2
			display.drawLine(x0, lineY, display.width()-1, lineY, 0x000000)
			display.drawText(x0+5, display.height()-pixHeight*2, consts.INFO_FIRMWARE_NAME, 0x000000, "roboto_regular12")
			display.drawText(x0+5, display.height()-pixHeight, "Build "+str(consts.INFO_FIRMWARE_BUILD), 0x000000, "roboto_regular12")
	else:
		pixHeight = display.getTextHeight(" ", "roboto_regular12")
		display.drawLine(0, display.height()-18*4, display.width(), display.height()-18*4, ugfx.BLACK)
		display.drawText(0, display.height()-pixHeight*6, "A: Run\n", 0x000000, "roboto_regular12")
		display.drawText(0, display.height()-pixHeight*5, "START: Home\n", 0x000000, "roboto_regular12")
		display.drawText(0, display.height()-pixHeight*4, "SELECT: Uninstall\n", 0x000000, "roboto_regular12")
		
		lineY = display.height()-pixHeight*2-pixHeight//2
		display.drawLine(0, lineY, display.width()-1, lineY, 0x000000)
		display.drawText(0, display.height()-pixHeight*2, consts.INFO_FIRMWARE_NAME, 0x000000, "roboto_regular12")
		display.drawText(0, display.height()-pixHeight, "Build "+str(consts.INFO_FIRMWARE_BUILD), 0x000000, "roboto_regular12")

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
	ugfx.input_attach(ugfx.BTN_START, input_start)

	populate_apps()
	populate_category()
	populate_options()

	# do a greyscale flush on start
	ugfx.flush(ugfx.GREYSCALE)

start()

def goToSleep(unused_variable=None):
	system.home()

# Read configuration from NVS or apply default values
cfg_term_menu = machine.nvs_get_u8('splash', 'term_menu') # Show a menu on the serial port instead of a prompt
if cfg_term_menu == None:
	cfg_term_menu = True # If not set the menu is shown

# Scheduler
virtualtimers.activate(100) # Start scheduler with 100ms ticks
virtualtimers.new(100, updateEink)

# Terminal menu
if cfg_term_menu:
	init_power_management()
	umenu = term_menu.UartMenu(system.home, pm, False, "< Back")
	umenu.main()

#(Note: power management is disabled when the menu is disabled, to keep the python prompt clean and usefull)

term.header(True, "Python shell")
print("Type \"import menu\" to access the menu.")
