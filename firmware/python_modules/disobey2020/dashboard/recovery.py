import display, orientation, term, term_menu, sys, ujson, system, buttons, machine, os

def drawMessageBox(text):
	width = display.getTextWidth(text, "org18")
	height = display.getTextHeight(text, "org18")
	display.drawRect((display.width()-width-4)//2, (display.height()-height-4)//2, width+2, height+2, True, 0xFFFFFF)
	display.drawText((display.width()-width)//2, (display.height()-height)//2-2, text, 0x000000, "org18")

def drawApp(app, position, amount):
	display.drawFill(0x000000)
	display.drawRect(0,0,display.width(), 10, True, 0xFFFFFF)
	display.drawText(2, 0, "RECOVERY", 0x000000, "org18")
	positionText = "{}/{}".format(position+1, amount)
	if app["path"].startswith("/sd"):
		positionText  = "SD "+positionText
	positionWidth = display.getTextWidth(positionText, "org18")
	display.drawText(display.width()-positionWidth-2, 0, positionText, 0x000000, "org18")
	titleWidth = display.getTextWidth(app["name"], "7x5")
	titleHeight = display.getTextHeight(app["name"], "7x5")
	display.drawText((display.width()-titleWidth)//2,(display.height()-titleHeight)//2, app["name"], 0xFFFFFF, "7x5")

	if not position < 1:
		display.drawText(0, display.height()//2-12, "<", 0xFFFFFF, "roboto_regular18")	
	if not position >= (amount-1):
		display.drawText(display.width()-10, display.height()//2-12, ">", 0xFFFFFF, "roboto_regular18")
	
	display.flush()

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
		display.drawFill(0x000000)
		drawMessageBox("Loading app...")
		display.flush()
		system.start(apps[currentApp]["path"])

def onB(pressed):
	pass

# Launcher
orientation.default()
display.drawFill(0x000000)
drawMessageBox("Loading...")
display.flush()
term.header(True, "Loading...")
apps = []
apps.append({"path":"dashboard.tools.reset_default",   "name":"Reset homescreen",   "icon":None, "category":"system"})
apps.append({"path":"dashboard.tools.battery_monitor", "name":"Battery monitor",    "icon":None, "category":"system"})
apps.append({"path":"dashboard.settings.wifi",         "name":"WiFi setup",         "icon":None,  "category":"system"})
apps.append({"path":"dashboard.tools.update_firmware", "name":"Update firmware",    "icon":None, "category":"system"})
apps.append({"path":"dashboard.tools.force_ota",       "name":"Force OTA update",   "icon":None, "category":"system"})
apps.append({"path":"dashboard.installer",             "name":"Installer",          "icon":None, "category":"system"})
apps.append({"path":"dashboard.tools.uninstall",       "name":"Remove an app",      "icon":None,     "category":"system"})
apps.append({"path":"dashboard.tools.update_apps",     "name":"Update apps",        "icon":None, "category":"system"})
apps.append({"path":"dashboard.home",                  "name":"Home",               "icon":None,      "category":"system"})

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

