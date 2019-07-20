import sys, uos as os, time, ujson
import machine, system, term_menu, virtualtimers, tasks.powermanagement as pm, buttons, defines, wifi
import rgb
from default_icons import icon_snake, icon_clock, icon_settings, icon_appstore, icon_activities, icon_nickname, \
                            icon_no_wifi, animation_connecting_wifi

# Application list

apps = []
current_index = 0
b_down = False


def show_text(text):
    rgb.scrolltext(text, (255,255,255))


def show_app_name(name):
    rgb.scrolltext(name, (255,255,255), (8,0), rgb.PANEL_WIDTH-8)


def clear():
    rgb.clear()


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
    try:
        icon = information["icon"]
        data, num_frames = icon
        if len(data) != 8 * 8 * num_frames:
            print('not equal')
            icon = icon_snake
    except:
        icon = icon_snake

    info = {"file": app, "title": title, "category": category, "icon": icon}
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
    add_app("snake", {"name": "Snake", "category": "system", "icon": icon_snake})
    add_app("clock", {"name": "Clock", "category": "system", "icon": icon_clock})
    # add_app("installer", {"name": "App store", "category": "system", "icon": icon_appstore})
    add_app("setupwifi", {"name": "Set up wifi", "category": "system", "icon": icon_settings})
    add_app("forceupdate", {"name": "Force OTA update", "category": "system", "icon": icon_settings})
    # add_app("update", {"name": "Update apps", "category": "system", "icon": icon_settings})
    # add_app("checkforupdates", {"name": "Update firmware", "category": "system", "icon": icon_nickname})


def render_current_app():
    clear()
    app = apps[current_index]
    data, num_frames = app["icon"]

    rgb.gif(data, (0, 0), (8, 8), num_frames)
    show_app_name(app["title"])


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

    # TODO: add removal dialog again
    # dialogs.prompt_boolean('Remove %s?' % currentListTitles[selected], cb=perform_uninstall)


# Run app

def run():
    system.start(apps[current_index]["file"], status=True)


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
def input_A(pressed):
    pm.feed()
    if pressed:
        run()


def input_B(pressed):
    global b_down
    pm.feed()
    b_down = pressed



def input_up(pressed):
    global current_index

    pm.feed()
    if pressed:
        current_index = (current_index - 1) % len(apps)
        render_current_app()


def input_down(pressed):
    global current_index

    pm.feed()
    if pressed:
        current_index = (current_index + 1) % len(apps)
        render_current_app()


def input_left(pressed):
    global current_index

    pm.feed()
    if pressed:
        rgb.setbrightness(rgb.getbrightness() - 2)


def input_right(pressed):
    global current_index

    pm.feed()
    if pressed:
        rgb.setbrightness(rgb.getbrightness() + 2)


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

    buttons.register(defines.BTN_A, input_A)
    buttons.register(defines.BTN_B, input_B)
    buttons.register(defines.BTN_UP, input_up)
    buttons.register(defines.BTN_DOWN, input_down)
    buttons.register(defines.BTN_LEFT, input_left)
    buttons.register(defines.BTN_RIGHT, input_right)

    populate_apps()
    render_current_app()

# First time boot sequence
if not machine.nvs_getint("system", 'intro_shown'):
    import nyan
    time.sleep(3)
    rgb.clear()
    time.sleep(1)
    import sponsors
    machine.nvs_setint("system", 'intro_shown', 1)
    system.reboot()

# Do shameless start-of-event update
if not machine.nvs_getint("system", 'day0_updated'):
    data, size, frames = animation_connecting_wifi
    rgb.clear()
    rgb.framerate(3)
    rgb.gif(data, (12, 0), size, frames)
    if wifi.connect():
        rgb.clear()
        rgb.framerate(20)
        rgb.setfont(1)
        rgb.scrolltext("Updating, don't remove battery")
        time.sleep(5)
        machine.nvs_setint("system", 'day0_updated', 1)
        system.ota()
    else:
        print('Need to perform day0 update, but no WiFi connection present')
        rgb.clear()
        rgb.framerate(20)
        data, frames = icon_no_wifi
        rgb.image(data, (12, 0), (8,8))
        time.sleep(3)
        rgb.clear()

start()
init_power_management()

menu = term_menu.UartMenu(None, pm)
menu.main()