import sys, uos as os, time, ujson, gc, term, deepsleep
import machine, system, term_menu, virtualtimers, tasks.powermanagement as pm, buttons, defines, woezel
import rgb, uinterface, uinstaller
from default_icons import icon_snake, icon_clock, icon_settings, icon_appstore, icon_activities, icon_nickname, \
                            icon_slider, icon_unknown

# Application list

apps = []
current_index = 0
current_icon = None


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
        if category == "system":
            icon = {'data': information["icon"]}
        else:
            icon = {'path': '%s/%s/icon' % (install_path, app)}

    except:
        icon = {'data': icon_unknown}

    info = {"file": app, "title": title, "category": category, "icon": icon}
    apps.append(info)


def populate_apps():
    global apps, current_index
    apps = []
    try:
        userApps = os.listdir('apps')
        userApps.reverse()
    except OSError:
        userApps = []
    for app in userApps:
        add_app(app, read_metadata(app))
    add_app("snake", {"name": "Snake", "category": "system", "icon": icon_snake})
    # add_app("activities", {"name": "Activities", "category": "system", "icon": icon_activities})
    add_app("clock", {"name": "Clock", "category": "system", "icon": icon_clock})
    add_app("nickname", {"name": "Nickname", "category": "system", "icon": icon_nickname})
    add_app("slider", {"name": "Slider", "category": "system", "icon": icon_slider})
    add_app("appstore", {"name": "App store", "category": "system", "icon": icon_appstore})
    add_app("setupwifi", {"name": "Set up wifi", "category": "system", "icon": icon_settings})
    add_app("slider_config", {"name": "Slider settings", "category": "system", "icon": icon_settings})
    add_app("update", {"name": "Firmware update", "category": "system", "icon": icon_settings})
    add_app("updateapps", {"name": "App updates", "category": "system", "icon": icon_settings})

    current_index = machine.nvs_getint('launcher', 'index') or 0
    if current_index >= len(apps):
        current_index = 0


def get_icon(app):
    try:
        if 'data' in app['icon']:
            return(app['icon']['data'])
        else:
            icon = __import__(app['icon']['path']).icon

            if len(icon) != 2:
                print('App icon for app "%s" isn\'t a tuple with a pixel array and the number of frames' % app['name'])
                return icon_unknown

            data, num_frames = icon
            if len(data) != 8 * 8 * num_frames:
                print('App icon for app "%s" is not 8*8 or has more/less frames than it says' % app['name'])
                return icon_unknown

            return icon
    except:
        return icon_unknown



def render_current_app():
    global current_icon
    clear()
    app = apps[current_index]

    current_icon = None
    gc.collect()
    current_icon = get_icon(app)

    data, num_frames = current_icon

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
        uinterface.skippabletext("System apps can't be removed")
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
        machine.nvs_setint('launcher', 'index', current_index)
        render_current_app()


def input_down(pressed):
    global current_index

    pm.feed()
    if pressed:
        current_index = (current_index + 1) % len(apps)
        machine.nvs_setint('launcher', 'index', current_index)
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
def cbSleep(_):
    rgb.clear()
    term.header(True, "Going to sleep...")
    uinterface.skippabletext('ZzZz')

def init_power_management():
    virtualtimers.activate(1000)  # Start scheduler with 1 second ticks
    pm.set_timeout(5 * 60 * 1000)  # Set timeout to 5 minutes
    pm.callback(cbSleep)  # Show sleep message
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

start()
init_power_management()

# Install CZ countdown app to replace activities app
# if not machine.nvs_getint('system', 'czcount_inst'):
#     if woezel.is_installed('campzone_2020_countdown'):
#         machine.nvs_setint('system', 'czcount_inst', 1)
#     else:
#         if uinterface.connect_wifi():
#             uinstaller.install('campzone_2020_countdown')
#
#     render_current_app()

cfg_term_menu = machine.nvs_get_u8('splash', 'term_menu')
if cfg_term_menu == None:
        cfg_term_menu = True

if cfg_term_menu:
    menu = term_menu.UartMenu(deepsleep.start_sleeping, pm)
    print(gc.mem_free())
    menu.main()
else:
        print("Welcome!")
        print("Press CTRL+C to reboot directly to a Python prompt.")
        wait = True
        while wait:
                c = machine.stdin_get(1,1)
                if c == "\x03" or c == "\x04": # CTRL+C or CTRL+D
                        wait = False
        import shell
