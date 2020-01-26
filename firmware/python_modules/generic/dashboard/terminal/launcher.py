import term, term_menu, sys, ujson, system, machine, os

haveSD = False
try:
	os.listdir("/sd")
	haveSD = True
except:
	pass

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

term.header(True, "Loading...")
apps = listApps()
amountOfUserApps = len(apps)
apps.append({"path":"dashboard.home", "name":"Home", "icon":None, "category":"system"})
apps.append({"path":"dashboard.installer", "name":"Installer", "icon":None, "category":"system"})
if amountOfUserApps > 0:
	apps.append({"path":"dashboard.tools.uninstall", "name":"Remove an app", "icon":None, "category":"system"})
	apps.append({"path":"dashboard.tools.update_apps", "name":"Update apps", "icon":None, "category":"system"})
if haveSD:
	apps.append({"path":"dashboard.tools.movetosd", "name":"Move from/to SD", "icon":None, "category":"system"})
apps.append({"path":"dashboard.tools.update_firmware", "name":"Update firmware", "icon":None, "category":"system"})
apps.append({"path":"dashboard.settings.nickname", "name":"Configure nickname", "icon":None, "category":"system"})
apps.append({"path":"dashboard.settings.wifi", "name":"WiFi setup", "icon":None, "category":"system"})
apps.append({"path":"dashboard.other.about", "name":"About", "icon":None, "category":"system"})

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
system.start(apps[start]["path"], True)
