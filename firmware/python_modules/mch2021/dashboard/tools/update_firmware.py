import term, system, time, version, wifi, buttons, orientation, display
import tasks.otacheck as otacheck
import easydraw
orientation.default()

term.header(True, "Update check")
print("Checking for updates...")
print("")
print("Currently installed:",version.name,"(Build "+str(version.build)+")")

available = 0

easydraw.messageCentered("Connecting to WiFi...", False, "/media/wifi.png")
wifi.connect()
wifi.wait()

if not wifi.status():
    easydraw.messageCentered("No WiFi!\n\nPress A to start update\nPress B to cancel", False)
    title = "Could not connect to WiFi!"
    message  = "Unable to check for updates. You can still attempt to start the OTA update procedure.\n"
    message += "Currently installed: "+version.name+" (Build "+str(version.build)+")\n"
else:
    info = otacheck.download_info()
    if info:
            print("Server has: ",info['name']," (Build "+str(info['build'])+")")
            if info["build"] > version.build:
                easydraw.messageCentered("Update available!\n({})\n\nPress A to start update\nPress B to cancel".format(version.build), False)
                message  = "A new firmware version is available. Update?\n"
                message += "Currently installed: "+version.name+" (Build "+str(version.build)+")\n"
                message += "Available          : "+info["name"]+" (Build "+str(info["build"])+")"
            elif info["build"] < version.build:
                easydraw.messageCentered("Downgrade available!\n({})\n\nPress A to start update\nPress B to cancel".format(version.build), False)
                title = "Firmware downgrade available"
                message  = "An older firmware version is available. Update?\n"
                message += "Currently installed: "+version.name+" (Build "+str(version.build)+")\n"
                message += "Available          : "+info["name"]+" (Build "+str(info["build"])+")"
            else:
                easydraw.messageCentered("Your badge is up-to-date!\n\nPress A to start update\nPress B to cancel", False)
                title = "Up-to-date"
                message = "You are up-to-date.\n"
                message += "Currently installed: "+version.name+" (Build "+str(version.build)+")\n"
                message += "Available          : "+info["name"]+" (Build "+str(info["build"])+")"
    else:
        easydraw.messageCentered("Unable to check for states\n\nPress A to start update\nPress B to cancel", False)
        title = "Update check"
        message = "An error occured while fetching information. You can still choose to start the OTA procedure."

def start(pressed):
    if pressed:
        system.ota()
        
def cancel(pressed):
    if pressed:
        system.launcher()

buttons.attach(buttons.BTN_A, start)
try:
    buttons.attach(buttons.BTN_START, cancel)
except:
    pass
buttons.attach(buttons.BTN_B, cancel)

items = ["Cancel", "Start OTA update"]
callbacks = [system.home, system.ota]
callbacks[term.menu(title, items, 0, message)](True)
