import tasks.otacheck as otacheck, easydraw, term, system, time, version, wifi, buttons, orientation, display

orientation.default()

term.header(True, "Update check")
print("Checking for updates...")
print("")
print("Currently installed:",version.name,"(Build "+str(version.build)+")")

available = 0

def start(pressed):
	if pressed:
		system.ota()
		
def cancel(pressed):
	if pressed:
		system.launcher()

easydraw.messageCentered("PLEASE WAIT\nConnecting to WiFi...", True, "/media/wifi.png")
wifi.connect()
wifi.wait()

easydraw.msg("Welcome!", "Firmware update", True)

title = "Update check"
message = "Unknown state!"
		
if wifi.status():
	info = otacheck.download_info()
	if info:
			print("Server has: ",info['name']," (Build "+str(info['build'])+")")
			if info["build"] > version.build:
				easydraw.messageCentered("UPDATE?\n("+info["name"]+")\nPress OK to update\nPress BACK to cancel", False)
				title = "Firmware update available"
				message  = "A new firmware version is available. Update?\n"
				message += "Currently installed: "+version.name+" (Build "+str(version.build)+")\n"
				message += "Available          : "+info["name"]+" (Build "+str(info["build"])+")"
			elif info["build"] < version.build:
				easydraw.messageCentered("DOWNGRADE?\n("+info["name"]+")\n\nPress OK to update\nPress BACK to cancel", False)
				title = "Firmware downgrade available"
				message  = "An older firmware version is available. Update?\n"
				message += "Currently installed: "+version.name+" (Build "+str(version.build)+")\n"
				message += "Available          : "+info["name"]+" (Build "+str(info["build"])+")"
			else:
				easydraw.messageCentered("Up-to-date!\n("+info["name"]+")\n\nPress OK to update\nPress BACK to cancel", False)
				title = "Up-to-date"
				message = "You are up-to-date.\n"
				message += "Currently installed: "+version.name+" (Build "+str(version.build)+")\n"
				message += "Available          : "+info["name"]+" (Build "+str(info["build"])+")"
	else:
		easydraw.messageCentered("Unknown status!\n\n\nPress OK to update\nPress BACK to cancel", False)
		title = "Update check"
		message = "An error occured while fetching information. You can still choose to start the OTA procedure."
else:
	easydraw.messageCentered("No WiFi available!\n\n\nPress OK to update\nPress BACK to cancel", False)
	title = "Update check"
	message = "Could not connect to the WiFi network. You can still choose to start the OTA procedure."

buttons.attach(buttons.BTN_OK, start)
buttons.attach(buttons.BTN_BACK, cancel)

items = ["Cancel", "Start OTA update"]
callbacks = [system.home, system.ota]
callbacks[term.menu(title, items, 0, message)](True)
