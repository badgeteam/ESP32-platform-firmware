import sys, uos as os, time, ujson, gc, deepsleep
import machine, system, term_menu, virtualtimers, tasks.powermanagement as pm, buttons, defines, woezel
import rgb, uinterface
from default_icons import icon_snake, icon_clock, icon_settings, icon_appstore, icon_activities, icon_nickname, \
                            icon_unknown

# Application list

apps = []
current_index = 0


def show_text(text):
    rgb.scrolltext(text, (255,255,255))


def show_app_name(name):
    rgb.scrolltext(name, (255,255,255), (8,0), rgb.PANEL_WIDTH-8)


def clear():
    rgb.clear()


def add_app(app, information):
    global apps
    install_path = woezel.get_install_path()
    try:
        title = information["name"]
    except:
        title = app
    try:
        category = information["category"]
    except:
        category = ""
    try:
        icon = information["icon"] if category == "system" else __import__('%s/%s/icon' % (install_path, app)).icon

        data, num_frames = icon
        if len(data) != 8 * 8 * num_frames:
            print('App icon for app "%s" is not 8*8 or has more/less frames than it says' % title)
            icon = icon_unknown

    except:
        icon = icon_unknown

    info = {"file": app, "title": title, "category": category, "icon": icon}
    apps.append(info)


def populate_apps():
    global apps
    apps = []
    try:
        userApps = os.listdir('apps')
    except OSError:
        userApps = []
    for app in userApps:
        add_app(app, read_metadata(app))
    add_app("snake", {"name": "Snake", "category": "system", "icon": icon_snake})
    add_app("activities", {"name": "Activities", "category": "system", "icon": icon_activities})
    add_app("clock", {"name": "Clock", "category": "system", "icon": icon_clock})
    add_app("nickname", {"name": "Nickname", "category": "system", "icon": icon_nickname})
    add_app("appstore", {"name": "App store", "category": "system", "icon": icon_appstore})
    add_app("setupwifi", {"name": "Set up wifi", "category": "system", "icon": icon_settings})
    add_app("update", {"name": "Firmware update", "category": "system", "icon": icon_settings})
    add_app("updateapps", {"name": "App updates", "category": "system", "icon": icon_settings})


def render_current_app():
    clear()
    app = apps[current_index]
    data, num_frames = app["icon"]

    rgb.gif(data, (0, 0), (8, 8), num_frames)
    show_app_name(app["title"])


# Read app metadata
def read_metadata(app):
    try:
        install_path = woezel.get_install_path()
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
def uninstall(app):
    if app["category"] == "system":
        # dialogs.notice("System apps can not be removed!","Can not uninstall '"+currentListTitles[selected]+"'")
        rgb.clear()
        rgb.scrolltext("System apps can't be removed")
        time.sleep(10)
        render_current_app()
        return

    machine.nvs_setstr('launcher', 'uninstall_name', app['title'])
    machine.nvs_setstr('launcher', 'uninstall_file', app['file'])
    system.start('uninstall')



# Run app

def run():
    system.start(apps[current_index]["file"], status=True)


# Path

def expandhome(s):
    if "~/" in s:
        h = os.getenv("HOME")
        s = s.replace("~/", h + "/")
    return s


# Actions        
def input_A(pressed):
    pm.feed()
    if pressed:
        run()


def input_B(pressed):
    pm.feed()
    if pressed:
        app = apps[current_index]
        uninstall(app)


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

    buttons.init_button_mapping()
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

start()
init_power_management()

menu = term_menu.UartMenu(deepsleep.start_sleeping, pm)
menu.main()