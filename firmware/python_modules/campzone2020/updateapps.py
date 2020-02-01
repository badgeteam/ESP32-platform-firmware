import system, time, uos as os, gc
import woezel, rgb, uinterface

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
    gc.collect()
    woezel.install(app)

rgb.clear()
uinterface.skippabletext('Done updating')
system.reboot()