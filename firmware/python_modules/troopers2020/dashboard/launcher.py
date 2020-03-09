import display, orientation, term, term_menu, sys, ujson, system, buttons, machine, os, time, _device as device

haveSD = False
try:
	os.listdir("/sd")
	haveSD = True
except:
	pass

default_icon = b'\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00 \x00\x00\x00 \x01\x00\x00\x00\x00[\x01GY\x00\x00\x00nIDAT\x08\xd7U\xce\xb1\r\x800\x0c\x05\xd1\x1fQ\xa4\xcc\x02\x88\xacAEVb\x02\xcchH\x14\xac\x01b\x00R\xa6\xb0l\x1cRq\xc5\xab\x0f\xf8\n\xd9 \x01\x9c\xf8\x15]q\x1b|\xc6\x89p\x97\x19\xf1\x90\x11\xf11\x92j\xdf \xd5\xe1\x87L\x06W\xf2b\\b\xec\x15_\x89\x82$\x89\x91\x98\x18\x91\xa94\x82j\x86g:\xd11mpLk\xdbhC\xd6\x0b\xf2 ER-k\xcb\xc6\x00\x00\x00\x00IEND\xaeB`\x82'

home_icon = b'\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00 \x00\x00\x00 \x01\x03\x00\x00\x00I\xb4\xe8\xb7\x00\x00\x00\x06PLTE\xff\xff\xff\x00\x00\x00U\xc2\xd3~\x00\x00\x00\tpHYs\x00\x00\x13\xaf\x00\x00\x13\xaf\x01c\xe6\x8e\xc3\x00\x00\x00\x19tEXtSoftware\x00www.inkscape.org\x9b\xee<\x1a\x00\x00\x00[IDAT\x08\x99\x8d\xce!\x0e\xc5 \x00\x04\xd1qH\x8e\xc0Q8Ze\xcf\xd5Tp\r\x1a\x04\x16\x89 \xddn\x93~\xffG<=\xc8\xa1{3+\x9b\x99L\r\xe68\xcd~\x998\xc4J33\xb7;1\xa4\xc8%\xed4\xa9\xd0\xa5\xfeQ\xc3\x8fV\x8c\xfb\x9b\xd6{\xa1B`@dA\xe6]{\x00\xb4\x17e\x0cD\xcab!\x00\x00\x00\x00IEND\xaeB`\x82'

trash_icon = b'\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00 \x00\x00\x00 \x01\x03\x00\x00\x00I\xb4\xe8\xb7\x00\x00\x00\x06PLTE\xff\xff\xff\x00\x00\x00U\xc2\xd3~\x00\x00\x00\tpHYs\x00\x00\x13\xaf\x00\x00\x13\xaf\x01c\xe6\x8e\xc3\x00\x00\x00\x19tEXtSoftware\x00www.inkscape.org\x9b\xee<\x1a\x00\x00\x00VIDAT\x08\x99\x95\xce\xb1\r\x80 \x14\x84\xe13\x16\x8c\xc1(\x8c&\x1b\xbc\x91t\x03F\x80\x15\x08\r\t\xe8y\x89\x95\xb1\xf2/\xbe\xf2r\xa0\x02\xbb\x17\xc5\x89c\x15\xa9\t\xab\xc4iuG\x01\xdcC\xcf\xc3\xa3\x93/F\x16\xd5~\xd0\xd2ge\x92\x01\x13\x08\xb8\x80\r\x8c\x8b\x1e\xa8\x1bL\xedW\xf4`=\x10\x0f\x00\x00\x00\x00IEND\xaeB`\x82'

settings_icon = b'\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00 \x00\x00\x00 \x01\x00\x00\x00\x00[\x01GY\x00\x00\x00.IDAT\x08\xd7c`@\x05<\x8c\r\x06\x84\x08\xf9\x1f0\x82\x81\x01J0\x80\t\x90,\x8c``\xa8o\x84\x11<\x8c\x8d\xff`\x04A\xe3Q\x01\x00\x83\xa4\x15\xa0iq3@\x00\x00\x00\x00IEND\xaeB`\x82'

installer_icon = b'\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00 \x00\x00\x00 \x08\x04\x00\x00\x00\xd9s\xb2\x7f\x00\x00\x02+IDATH\xc7\xc5\x95\xbfKUa\x18\xc7?\xe7\\+J3\x1b\xa4_C&]\x82\xa0\xd2\xa0\x1aB\x0c\x97\xa0h\xe9&\x91\x0eE\x97\x8c\x06\xa1\xa1?\xc0\xa2\xad\xa8\xad\xc5@\\\xa4\xa1\xc1,\x82\x82 h\x08#\x88\x86\xca\x88~aXP\xd8P\x0e\xa6\xc7O\xc3=\xeaU\xcf\xb9\x17s\xe8y\xe1\xc0\xfb<\xcf\xf7{\xde\xe7}~\xbc\x01\xb2$\t\x97\x06\x87\x8a\x85*\xab\xc8\xb2\x851\xbe\xf31\xf8Y\x9e\xc2\xd9\xe5j\xcf8\xe8\xacL\xf8\xc8N+\x8b}\x16\xac"x\xbb\xa3&\xc9\x17\xdb\x0c\xd2\x08\x82\xc2%ZI/\xc7\x00\x18\xe7>/\xf8\xc0r\xd6\xd1L\x0b\xcb\x01\xe8\xe3T\xf0\'5\x04\x97yO\xd5q/Z;\'\xa8\x1a\xaf8\xae\xea\x1dW\xa4\x86\xe0MU\xdf\xdb\x90\xe4\xe2.GT\xbd\x9aB`\x93S\xea\x88\xf5iq\xba\xd5\xafj\xe4\xfe$k\xe8sU\x9bK\xdd\xb5GT}\x9ad\xdb\xa7\xea\xed\x92\xa9\x12\xedW\xb5\xc3\xc3\xee4,\xb6\\V\xf5@Y\x82CE\x89\xfd\xe6%\xab\xa6-\x83\xea\xa8\x15e\t2\xbe\x9bS\x1d\x9f\xdc\x81X\xc1\x06\xe0u0Q\xae`\x83I\xd7\xd3D\xc0Jv\xd3F-\x9by\xec\xb6\xe0-\xfeV\xef\xa6\xfe\xb7\xd1\xc6\x04m\xb5\xb7T}f\x88\xbf\xd4\x07)\xf0\x06#\xa3\xa4\xea0\xe3CU\x8f\x86\x0c\x01\xdbS\xce\x9d%$$\x9b\x14\x10\x9d\x08\xb4\x87\xbc\x016\x99e\x91\x12\x0c\xf1\x12\xd8\x1b\xd2\x0f\x04t\xfc\xc3,\xf9\x0c\xd4\x86\x0c0\x02\x9c\xb5n\xd1\x045\xc0X\x18Lp\r\xa8\xa6\xd7\xccb\xd0\xaea\x0f\xf0*\x04\xae\xf3\x04h\xa6%6\xd5\xdbcn\x01 g\x8f\xf5E\x8a\x0b\xac\x02\x06\n\xed\\\xe7\xb0\xc3n\x8cS\xd4\xadF\xe6\xd1VU[\xd1\xbc\x91\xda=\x93\xc6\x13N\xaa?\\KB\x8esF\x05\x8ai\x82\x18\x1e\x99\x8b=\x0e\x1a\xa9S\x1e\x9f3\x13\x8b(N\xc7\x0e\xdd\xaa\xde\x88w\xe7f\xecy5\xf2\xfc\xbc\xa1\x9aHa\xfc-\x82\xa3\x19O\xda\xb4`*\xcf\xa3(\x1c\xbb \x91\xf9\xd4>-\xd1\xc0\xf9\x99\x13\xa4\xc3K\x11\xa0]\xaav\x95\xf2\x89\xdf\x85\xd4ri\x83\xa0\xafdO\xf0\xbf_\xe7\xbf\x9c?\x12\x1e\x14Zz\xba\x00\x00\x00\x00IEND\xaeB`\x82'

about_icon = b"\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00 \x00\x00\x00 \x01\x00\x00\x00\x00[\x01GY\x00\x00\x00_IDAT\x08\xd7m\x8e\xb1\x11\x80 \x10\x04\x97\xf9\xc0\x90\x0e\xa4\x11\x86\xba\x0c\x98\x91\xd2\xbe\x14K $\x12\x99\xd3\xd0\x0f6\xfa\xdb;\xd0\xa5\xb1p\xde\x10\xe6l\xd8\x9c\xce\xd6\xe3Et\xeb\xa4\x16\x06\t\x06\xc5\xbc\x92\xcd3\xd5\xbcp\xc0\xfe!\xb4\x05\xf3\x7f\xe8Y1\t\xa4\x92Tz\x15\xa9R\xe5\x9aA\xea\xef\xb0\x07''-\xb9\x1b@\xfa\xf4\x00\x00\x00\x00IEND\xaeB`\x82"

nickname_icon = b'\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00 \x00\x00\x00 \x01\x03\x00\x00\x00I\xb4\xe8\xb7\x00\x00\x00\x06PLTE\x00\x00\x00\xff\xff\xff\xa5\xd9\x9f\xdd\x00\x00\x00\tpHYs\x00\x00\x16%\x00\x00\x16%\x01IR$\xf0\x00\x00\x00\x19tEXtSoftware\x00www.inkscape.org\x9b\xee<\x1a\x00\x00\x00FIDAT\x08\x99c`@\x03\xec\x0f\x80\x84\xfc\x0f !c\x01$,d\x80\x84\x01\x0f\x0e\x02\xac\x04\xac\x98\xff\x03L/\x1a\xf8\xff\x1f(\xfe\xff\xff\x03\x06\xf6\x06\xc6\x07\x0c\xfc\x0c\x0c\x1f\x18\xf8\x18\x18\n\xb0\x12@#\r\xd0\xb4\x03\x00\xfe}\x11 \x01G-\xe6\x00\x00\x00\x00IEND\xaeB`\x82'

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
				with open(app["path"]+"/"+app["icon"], "rb") as icon_file:
					icon_data = icon_file.read()
			else:
				icon_data = app["icon"]
		if not icon_data:
			display.drawPng(display.width()//2-16,display.height()//2-16,default_icon)
		else:
			info = display.pngInfo(icon_data)
			if info[0] == 32 and info[1] == 32:
				display.drawPng(display.width()//2-16,display.height()//2-16,icon_data)
			else:
				drawMessageBox("Invalid icon size\nExpected 32x32!")
	except BaseException as e:
		sys.print_exception(e)
		drawMessageBox("Icon parsing error!")
	
	if not position < 1:
		display.drawText(0, display.height()//2-12, "<", 0xFFFFFF, "roboto_regular18")	
	if not position >= (amount-1):
		display.drawText(display.width()-10, display.height()//2-12, ">", 0xFFFFFF, "roboto_regular18")
	
	display.flush(display.FLAG_LUT_FASTEST)

def loadInfo(folder, name):
	try:
		info_file = "{}/{}/metadata.json".format(folder, name)
		with open(info_file) as f:
			information = f.read()
		return ujson.loads(information)
	except BaseException as e:
		sys.print_exception(e)
		return {}

def listApps():
	apps = []
	for folder in sys.path:
		if folder != '':
			try:
				files = os.listdir(folder)
			except OSError:
				files = []
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
		device.showLoadingScreen(apps[currentApp]["name"])
		system.start(apps[currentApp]["path"])

def onB(pressed):
	if pressed:
		device.showLoadingScreen("Homescreen")
		system.home()

# Launcher
orientation.landscape()
display.drawFill(0x000000)
drawMessageBox("Loading...")
display.flush()
term.header(True, "Loading...")
apps = listApps()
amountOfUserApps = len(apps)
apps.append({"path":"dashboard.home",                  "name":"Home",               "icon":home_icon,      "category":"system"})
apps.append({"path":"dashboard.installer",             "name":"Installer",          "icon":installer_icon, "category":"system"})
if amountOfUserApps > 0:
	apps.append({"path":"dashboard.tools.uninstall",       "name":"Remove an app",      "icon":trash_icon,     "category":"system"})
	apps.append({"path":"dashboard.tools.update_apps",     "name":"Update apps",        "icon":installer_icon, "category":"system"})
if haveSD:
	apps.append({"path":"dashboard.tools.movetosd",        "name":"Move from/to SD",    "icon":settings_icon, "category":"system"})
apps.append({"path":"dashboard.tools.update_firmware", "name":"Update firmware",    "icon":installer_icon, "category":"system"})
apps.append({"path":"dashboard.settings.nickname",     "name":"Configure nickname", "icon":nickname_icon,  "category":"system"})
apps.append({"path":"dashboard.settings.wifi",         "name":"WiFi setup",         "icon":settings_icon,  "category":"system"})
apps.append({"path":"dashboard.other.about",           "name":"About",              "icon":about_icon,     "category":"system"})

currentApp = 0

buttons.attach(buttons.BTN_LEFT,  onLeft)
buttons.attach(buttons.BTN_RIGHT, onRight)
buttons.attach(buttons.BTN_A,     onA)
try:
	buttons.attach(buttons.BTN_START, onA)
except:
	pass
buttons.attach(buttons.BTN_B,     onB)

drawApp(apps[0],0,len(apps))

# Terminal menu
labels = []

for app in apps:
	label = app["name"]
	if app["path"].startswith("/sd"):
		label += " [SD card]"
	elif app["path"].startswith("/lib"):
		label += " [Legacy]"
	#elif app["path"].startswith("/apps"):
	#	label += " [Platform]"
	#else:
	#	label += " [Built-in]"
	labels.append(label)
labels.append("< Back")

start = term.menu("Launcher", labels, 0, "")
if start >= len(apps):
	system.home()
display.drawFill(0x000000)
drawMessageBox("Loading app...")
display.flush()
system.start(apps[start]["path"], True)
