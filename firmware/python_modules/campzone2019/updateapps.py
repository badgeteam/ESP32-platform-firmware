import system, time, uos as os
import woezel, rgb, uinterface
from default_icons import icon_no_wifi, animation_connecting_wifi, animation_loading

if not uinterface.connect_wifi():
    print("Error connecting to wifi")
    system.reboot()

try:
    apps = os.listdir('apps')
except OSError:
    apps = []

rgb.clear()
uinterface.loading_text('Updating:')
time.sleep(4)

for app in apps:
    rgb.clear()
    uinterface.loading_text(app)
    woezel.install(app)

rgb.clear()
rgb.scrolltext('Done updating')
time.sleep(6)
system.reboot()