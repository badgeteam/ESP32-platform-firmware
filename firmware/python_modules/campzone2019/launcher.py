import sys, uos as os, time, ujson
import appglue, virtualtimers, tasks.powermanagement as pm, dialogs, buttons, config
import rgb

# Application list

apps = []


def show_text(text):
    rgb.scrolltext(text, 0, 0, rgb.PANEL_WIDTH)


def add_app(app, information):
    global apps
    try:
        title = information["name"]
    except:
        title = app
    try:
        category = information["category"]
    except:
        category = ""
    info = {"file": app, "title": title, "category": category}
    apps.append(info)


def populate_apps():
    global apps
    apps = []
    try:
        userApps = os.listdir('lib')
    except OSError:
        userApps = []
    for app in userApps:
        add_app(app, read_metadata(app))
    add_app("installer", {"name": "Installer", "category": "system"})
    add_app("update", {"name": "Update apps", "category": "system"})
    add_app("checkforupdates", {"name": "Update firmware", "category": "system"})
    add_app(badge.nvs_get_str('boot', 'splash', 'splash'), {"name": "Home", "category": "system"})


# List as shown on screen
currentListTitles = []
currentListTargets = []


def populate_category(category="", system=True):
    global apps
    global currentListTitles
    global currentListTargets
    currentListTitles = []
    currentListTargets = []
    for app in apps:
        if (category == "" or category == app["category"] or (system and app["category"] == "system")) and (
        not app["category"] == "hidden"):
            currentListTitles.append(app["title"])
            currentListTargets.append(app)


def populate_options():
    global options
    options = ugfx.List(0, 0, int(ugfx.width()), ugfx.height())
    global currentListTitles
    for title in currentListTitles:
        options.add_item(title)


# Read app metadata
def read_metadata(app):
    try:
        install_path = get_install_path()
        info_file = "%s/%s/metadata.json" % (install_path, app)
        print("Reading " + info_file + "...")
        with open(info_file) as f:
            information = f.read()
        return ujson.loads(information)
    except BaseException as e:
        print("[ERROR] Can not read metadata for app " + app)
        sys.print_exception(e)
        information = {"name": app, "description": "", "category": "", "author": "", "revision": 0}
        return [app, ""]


# Uninstaller

def uninstall():
    global options
    selected = options.selected_index()
    options.destroy()

    global currentListTitles
    global currentListTargets

    if currentListTargets[selected]["category"] == "system":
        # dialogs.notice("System apps can not be removed!","Can not uninstall '"+currentListTitles[selected]+"'")
        show_text("System apps can not be removed!")
        time.sleep(2)
        start()
        return

    def perform_uninstall(ok):
        global install_path
        if ok:
            show_text("Removing " + currentListTitles[selected] + "...")
            install_path = get_install_path()
            for rm_file in os.listdir("%s/%s" % (install_path, currentListTargets[selected]["file"])):
                show_text("Deleting '" + rm_file + "'...")
                os.remove("%s/%s/%s" % (install_path, currentListTargets[selected]["file"], rm_file))
            show_text("Deleting folder...")
            os.rmdir("%s/%s" % (install_path, currentListTargets[selected]["file"]))
            show_text("Uninstall completed!")
        start()

    dialogs.prompt_boolean('Remove %s?' % currentListTitles[selected], cb=perform_uninstall)


# Run app

def run():
    global options
    selected = options.selected_index()
    options.destroy()
    global currentListTargets
    appglue.start_app(currentListTargets[selected]["file"])


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
def input_run(pressed):
    pm.feed()
    if pressed:
        run()



def input_uninstall(pressed):
    pm.feed()
    if pressed:
        uninstall()


def input_other(pressed):
    pm.feed()


# Power management
def pm_cb(dummy):
    appglue.home()


def init_power_management():
    virtualtimers.activate(1000)  # Start scheduler with 1 second ticks
    pm.set_timeout(5 * 60 * 1000)  # Set timeout to 5 minutes
    pm.callback(pm_cb)  # Go to splash instead of sleep
    pm.feed()  # Feed the power management task, starts the countdown...


# Main application
def start():
    global options
    global install_path
    options = None
    install_path = None

    buttons.register(config.BTN_A, input_run)
    buttons.register(config.BTN_UP, input_other)
    buttons.register(config.BTN_DOWN, input_other)
    buttons.register(config.BTN_LEFT, input_other)
    buttons.register(config.BTN_RIGHT, input_other)

    populate_apps()
    populate_category()
    populate_options()


start()
init_power_management()
