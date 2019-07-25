import system, time
import woezel, rgb, uinterface
from default_icons import icon_no_wifi, animation_connecting_wifi, animation_loading

def woezel_callback(text, error):
    global category_stats
    rgb.clear()
    rgb.framerate(20)
    rgb.setfont(rgb.FONT_7x5)

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
    elif 'Downloading categories...' in text:
        data, size, frames = animation_loading
        rgb.gif(data, (1, 1), size, frames)
        rgb.scrolltext('Loading', pos=(8,0), width=(rgb.PANEL_WIDTH - 8))
    elif 'Installing' in text:
        data, size, frames = animation_loading
        rgb.gif(data, (1, 1), size, frames)
        rgb.scrolltext(text, pos=(8,0), width=(rgb.PANEL_WIDTH - 8))
    elif "Downloading '" in text:
        cur, total = text.split('(')[1].split(')')[0].split('/')  # Definitely not proud of this
        progress = '(%s/%s)' % (cur, total)
        data, size, frames = animation_loading
        rgb.gif(data, (1, 1), size, frames)
        rgb.setfont(rgb.FONT_6x3)
        rgb.text(progress, pos=(8,1))
    elif 'Done!' in text or 'Failed!' in text:
        pass

woezel.set_progress_callback(woezel_callback)
categories = woezel.get_categories()
active_categories = [cat for cat in categories if cat['eggs'] > 0]

if len(active_categories) == 0:
    rgb.clear()
    rgb.framerate(20)
    rgb.scrolltext('Error loading')
    time.sleep(6)
    system.reboot()

while True:
    current_category = 0

    chosen_index = uinterface.menu([app['name'] for app in active_categories])
    if chosen_index is None:
        system.reboot()
    category = active_categories[chosen_index]

    apps = woezel.get_category(category['slug'])
    chosen_index = uinterface.menu([app['name'] for app in apps])
    if chosen_index is None:
        continue
    app = apps[chosen_index]

    # WiFi could have been disconnected by now
    uinterface.connect_wifi()

    if woezel.install(app['slug']):
        rgb.clear()
        rgb.scrolltext('Successfully installed')
        time.sleep(7)
    else:
        rgb.clear()
        rgb.scrolltext('Error installing')
        time.sleep(6)
    system.reboot()