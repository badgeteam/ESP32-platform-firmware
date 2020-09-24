import term, appglue, sys, badge, uos as os, ujson, easydraw

easydraw.msg("This app can only be controlled using the USB-serial connection.", "Notice", True)

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
			if not "author" in app:
				app["author"] = ""
			if not "revision" in app:
				app["revision"] = ""
			appDesc = app["title"]
			for i in range(0,32-len(app["title"])):
				appDesc += " "
			appDesc += " "+app["author"]
			for i in range(0,32-len(app["author"])):
				appDesc += " "
			appDesc += " (v"+app["revision"]+")"
			currentListTitles.append(appDesc)
			currentListTargets.append(app)
            
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
        return information

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

install_path = None

term.empty_lines()
term.header("Loading application list...")
populate_apps()
populate_category()

currentListTitles.append("< Back to the main menu")
selected = term.menu("Application launcher", currentListTitles)
if selected == len(currentListTitles) - 1:
	appglue.home()
else:
	appglue.start_app(currentListTargets[selected]['file'])
