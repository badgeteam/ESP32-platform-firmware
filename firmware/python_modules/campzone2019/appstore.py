import system, time, wifi
import woezel, rgb, uinterface
from default_icons import icon_no_wifi, animation_connecting_wifi, animation_loading

def woezel_callback(text, error):
    rgb.clear()
    rgb.framerate(20)

    print(text)

    if 'Connecting to WiFi' in text:
        data, size, frames = animation_connecting_wifi
        rgb.framerate(3)
        rgb.gif(data, (12, 0), size, frames)
    elif 'Failed to connect to WiFi' in text:
        data, frames = icon_no_wifi
        rgb.gif(data, (12, 0), (8, 8), frames)
        time.sleep(3)
        system.reboot()
    elif 'Downloading' in text or 'Installing' in text:
        app = text.replace("Downloading '", '').replace("'...", '')
        data, size, frames = animation_loading
        rgb.gif(data, (1, 1), size, frames)
        rgb.scrolltext(app, pos=(8,0), width=(rgb.PANEL_WIDTH - 8))
    elif 'Done!' in text or 'Failed!' in text:
        pass

woezel.set_progress_callback(woezel_callback)
categories = woezel.get_categories()

while True:
    chosen_index = uinterface.menu([app['name'] for app in categories])
    if chosen_index is None:
        system.reboot()
    category = categories[chosen_index]

    apps = woezel.get_category(category['slug'])
    chosen_index = uinterface.menu([app['name'] for app in apps])
    if chosen_index is None:
        continue
    app = apps[chosen_index]

    if woezel.install(app['slug']):
        rgb.scrolltext('Successfully installed')
        time.sleep(7)
    else:
        rgb.scrolltext('Error installing')
        time.sleep(3)
    system.reboot()