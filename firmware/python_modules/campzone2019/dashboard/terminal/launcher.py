import term, system, sys, uos as os, ujson
import woezel

system.serialWarning()

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
        userApps = os.listdir('apps')
    except OSError:
        userApps = []
    for app in userApps:
        add_app(app,read_metadata(app))

    add_app("snake", {"name": "Snake", "category": "system"})
    add_app("clock", {"name": "Clock", "category": "system"})
    add_app("nickname", {"name": "Nickname", "category": "system"})
    add_app("slider", {"name": "Slider", "category": "system"})
    add_app("appstore", {"name": "App store", "category": "system"})

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
            
def read_metadata(app):
    try:
        install_path = woezel.get_install_path()
        info_file = "%s/%s/metadata.json" % (install_path, app)
        print("Reading "+info_file+"...")
        with open(info_file) as f:
            information = f.read()
        return ujson.loads(information)
    except BaseException as e:
        print("[ERROR] Can not read metadata for app "+app)
        sys.print_exception(e)
        information = {"name":app,"title":"---", "category":""}
        return information

def expandhome(s):
    if "~/" in s:
        h = os.getenv("HOME")
        s = s.replace("~/", h + "/")
    return s

term.empty_lines()
term.header("Loading application list...")
populate_apps()
populate_category()

currentListTitles.append("< Back to the main menu")
selected = term.menu("Application launcher", currentListTitles)
if selected == len(currentListTitles) - 1:
    system.home()
else:
    system.start(currentListTargets[selected]['file'])
