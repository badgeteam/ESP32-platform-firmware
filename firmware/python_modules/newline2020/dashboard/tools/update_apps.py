import ugfx, woezel, easywifi, easydraw, system, time, os

def stop():
    time.sleep(2)
    system.launcher()

easydraw.msg("Welcome!","Updating apps...",True)


if not easywifi.status():
    if not easywifi.enable():
        stop()

try:
    apps = os.listdir('lib')
except OSError:
    easydraw.msg("There are no apps installed.")
    stop()

for app in apps:
    easydraw.msg("Updating '"+app+"'...")
    try:
        woezel.install(app)
        easydraw.msg("Done!")
    except:
        print("failed update. Already newest version?")

easydraw.msg("All your apps are now up-to-date!")
stop()
