import display, orientation, term, term_menu, sys, ujson, system, buttons, machine, os

default_icon = b'\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00 \x00\x00\x00 \x01\x00\x00\x00\x00[\x01GY\x00\x00\x00nIDAT\x08\xd7U\xce\xb1\r\x800\x0c\x05\xd1\x1fQ\xa4\xcc\x02\x88\xacAEVb\x02\xcchH\x14\xac\x01b\x00R\xa6\xb0l\x1cRq\xc5\xab\x0f\xf8\n\xd9 \x01\x9c\xf8\x15]q\x1b|\xc6\x89p\x97\x19\xf1\x90\x11\xf11\x92j\xdf \xd5\xe1\x87L\x06W\xf2b\\b\xec\x15_\x89\x82$\x89\x91\x98\x18\x91\xa94\x82j\x86g:\xd11mpLk\xdbhC\xd6\x0b\xf2 ER-k\xcb\xc6\x00\x00\x00\x00IEND\xaeB`\x82'

home_icon = b'\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00 \x00\x00\x00 \x01\x03\x00\x00\x00I\xb4\xe8\xb7\x00\x00\x00\x06PLTE\xff\xff\xff\x00\x00\x00U\xc2\xd3~\x00\x00\x00\tpHYs\x00\x00\x13\xaf\x00\x00\x13\xaf\x01c\xe6\x8e\xc3\x00\x00\x00\x19tEXtSoftware\x00www.inkscape.org\x9b\xee<\x1a\x00\x00\x00[IDAT\x08\x99\x8d\xce!\x0e\xc5 \x00\x04\xd1qH\x8e\xc0Q8Ze\xcf\xd5Tp\r\x1a\x04\x16\x89 \xddn\x93~\xffG<=\xc8\xa1{3+\x9b\x99L\r\xe68\xcd~\x998\xc4J33\xb7;1\xa4\xc8%\xed4\xa9\xd0\xa5\xfeQ\xc3\x8fV\x8c\xfb\x9b\xd6{\xa1B`@dA\xe6]{\x00\xb4\x17e\x0cD\xcab!\x00\x00\x00\x00IEND\xaeB`\x82'

trash_icon = b'\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00 \x00\x00\x00 \x01\x03\x00\x00\x00I\xb4\xe8\xb7\x00\x00\x00\x06PLTE\xff\xff\xff\x00\x00\x00U\xc2\xd3~\x00\x00\x00\tpHYs\x00\x00\x13\xaf\x00\x00\x13\xaf\x01c\xe6\x8e\xc3\x00\x00\x00\x19tEXtSoftware\x00www.inkscape.org\x9b\xee<\x1a\x00\x00\x00VIDAT\x08\x99\x95\xce\xb1\r\x80 \x14\x84\xe13\x16\x8c\xc1(\x8c&\x1b\xbc\x91t\x03F\x80\x15\x08\r\t\xe8y\x89\x95\xb1\xf2/\xbe\xf2r\xa0\x02\xbb\x17\xc5\x89c\x15\xa9\t\xab\xc4iuG\x01\xdcC\xcf\xc3\xa3\x93/F\x16\xd5~\xd0\xd2ge\x92\x01\x13\x08\xb8\x80\r\x8c\x8b\x1e\xa8\x1bL\xedW\xf4`=\x10\x0f\x00\x00\x00\x00IEND\xaeB`\x82'

def drawMessageBox(text):
	width = display.getTextWidth(text, "org18")
	height = display.getTextHeight(text, "org18")
	display.drawRect((display.width()-width-4)//2, (display.height()-height-4)//2, width+2, height+2, True, 0xFFFFFF)
	display.drawText((display.width()-width)//2, (display.height()-height)//2-2, text, 0x000000, "org18")

def drawApp(app, position, amount):
	display.drawFill(0x000000)
	display.drawRect(0,0,display.width(), 10, True, 0xFFFFFF)
	display.drawText(2, 0, "LAUNCHER", 0x000000, "org18")
	positionText = "{}/{}".format(position+1, amount)
	if app["path"].startswith("/sd"):
		positionText  = "SD "+positionText
	positionWidth = display.getTextWidth(positionText, "org18")
	display.drawText(display.width()-positionWidth-2, 0, positionText, 0x000000, "org18")
	titleWidth = display.getTextWidth(app["name"], "7x5")
	display.drawText((display.width()-titleWidth)//2,display.height()-10, app["name"], 0xFFFFFF, "7x5")
	try:
		icon_data = None
		if app["icon"]:
			if not app["icon"].startswith(b"\x89PNG"):
				with open(app["icon"], "rb") as icon_file:
					icon_data = icon_file.read()
			else:
				icon_data = app["icon"]
		if not icon_data:
			display.drawPng(48,15,default_icon)
		else:
			info = display.pngInfo(icon_data)
			if info[0] == 32 and info[1] == 32:
				display.drawPng(48,15,icon_data)
			else:
				drawMessageBox("Invalid icon size\nExpected 32x32!")
	except BaseException as e:
		sys.print_exception(e)
		drawMessageBox("Icon parsing error!")
	
	if not position < 1:
		display.drawText(0, display.height()//2-12, "<", 0xFFFFFF, "roboto_regular18")	
	if not position >= (amount-1):
		display.drawText(display.width()-10, display.height()//2-12, ">", 0xFFFFFF, "roboto_regular18")
	
	display.flush()

def loadInfo(folder, name):
	try:
		info_file = "%s/%s/metadata.json".format(folder, name)
		with open(info_file) as f:
			information = f.read()
		return ujson.loads(information)
	except BaseException as e:
		return None

def listApps():
	apps = []
	for folder in sys.path:
		if folder != '':
			try:
				files = os.listdir(folder)
			except OSError:
				files = []
				print("Folder {} could not be accessed.".format(folder))
			if len(files) < 1:
				print("Folder {} is empty.".format(folder))
			for name in files:
				hidden = False
				app = {"path":folder+"/"+name, "name":name, "icon":None, "category":"unknown"}
				metadata = loadInfo(folder, name)
				if metadata:
					if "name" in metadata:
						app["name"]     = metadata["name"]
					if "category" in metadata:
						app["category"] = metadata["category"]
					if "icon" in metadata:
						app["icon"] = metadata["icon"]
					if "hidden" in metadata:
						hidden = metadata["hidden"]
				if not hidden:
					apps.append(app)
				print("Found app {} in {}".format(app["name"], app["path"]))
	return apps

def onLeft(pressed):
	global currentApp, apps
	if pressed:
		currentApp -= 1
		if currentApp < 0:
			currentApp = len(apps)-1
		drawApp(apps[currentApp], currentApp, len(apps))

def onRight(pressed):
	global currentApp, apps
	if pressed:
		currentApp += 1
		if currentApp >= len(apps):
			currentApp = 0
		drawApp(apps[currentApp], currentApp, len(apps))

def onA(pressed):
	global currentApp, apps
	if pressed:
		system.start(apps[currentApp]["path"])

# Launcher
orientation.default()
term.header(True, "Loading...")
apps = listApps()
apps.append({"path":"dashboard.home",                  "name":"Home",               "icon":home_icon, "category":"system"})
apps.append({"path":"dashboard.installer",             "name":"Installer",          "icon":None, "category":"system"})
apps.append({"path":"dashboard.tools.uninstaller",     "name":"Remove an app",      "icon":trash_icon, "category":"system"})
apps.append({"path":"dashboard.settings.wifi",         "name":"WiFi configuration", "icon":None, "category":"system"})
apps.append({"path":"dashboard.tools.update_apps",     "name":"Update apps",        "icon":None, "category":"system"})
apps.append({"path":"dashboard.tools.update_firmware", "name":"Update firmware",    "icon":None, "category":"system"})
apps.append({"path":"dashboard.other.about",           "name":"About",              "icon":None, "category":"system"})

currentApp = 0

buttons.attach(buttons.BTN_LEFT, onLeft)
buttons.attach(buttons.BTN_RIGHT, onRight)
buttons.attach(buttons.BTN_A, onA)

drawApp(apps[0],0,len(apps))

# Read configuration from NVS or apply default values
cfg_term_menu = machine.nvs_get_u8('splash', 'term_menu') # Show a menu on the serial port instead of a prompt
if cfg_term_menu == None:
	cfg_term_menu = True # If not set the menu is shown

# Terminal menu
labels = []

for app in apps:
	labels.append(app["name"])
labels.append("< Back")

start = term.menu("Launcher", labels, 0, "")
if start >= len(apps):
	system.home()
system.start(apps[start]["path"])
