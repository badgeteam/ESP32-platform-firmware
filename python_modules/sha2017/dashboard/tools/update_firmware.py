import tasks.otacheck as otacheck, easydraw, term, system, time, version, wifi, ugfx, orientation, display

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
				print("Update available!")
				easydraw.msg(" ")
				easydraw.msg("An update is available!")
				easydraw.msg(" ")
				easydraw.msg_nosplit("Currently installed: "+version.name+" (Build "+str(version.build)+")")
				easydraw.msg_nosplit("Available          : "+info["name"]+" (Build "+str(info["build"])+")")
				easydraw.msg(" ")
				easydraw.msg("Press A to start the update")
				easydraw.msg("Press B to cancel")
				title = "Firmware update available"
				message  = "A new firmware version is available. Update?\n"
				message += "Currently installed: "+version.name+" (Build "+str(version.build)+")\n"
				message += "Available          : "+info["name"]+" (Build "+str(info["build"])+")"
			elif info["build"] < version.build:
				print("Server has an older version.")
				easydraw.msg(" ")
				easydraw.msg("A downgrade is available!")
				easydraw.msg(" ")
				easydraw.msg_nosplit("Currently installed: "+version.name+" (Build "+str(version.build)+")")
				easydraw.msg_nosplit("Available          : "+info["name"]+" (Build "+str(info["build"])+")")
				easydraw.msg(" ")
				easydraw.msg("Press A to start the downgrade")
				easydraw.msg("Press B to cancel")
				title = "Firmware downgrade available"
				message  = "An older firmware version is available. Update?\n"
				message += "Currently installed: "+version.name+" (Build "+str(version.build)+")\n"
				message += "Available          : "+info["name"]+" (Build "+str(info["build"])+")"
			else:
				print("You are up-to-date!")
				easydraw.msg(" ")
				easydraw.msg("Your badge is up-to-date!")
				easydraw.msg(" ")
				easydraw.msg_nosplit("Currently installed: "+version.name+" (Build "+str(version.build)+")")
				easydraw.msg_nosplit("Available          : "+info["name"]+" (Build "+str(info["build"])+")")
				easydraw.msg(" ")
				easydraw.msg("Press A to start the update anyway")
				easydraw.msg("Press B to cancel")
				title = "Up-to-date"
				message = "You are up-to-date.\n"
				message += "Currently installed: "+version.name+" (Build "+str(version.build)+")\n"
				message += "Available          : "+info["name"]+" (Build "+str(info["build"])+")"
	else:
		print("An error occured!")
		easydraw.msg(" ")
		easydraw.msg("An error occured while checking for available updates.")
		easydraw.msg(" ")
		easydraw.msg_nosplit("Currently installed: "+version.name+" (Build "+str(version.build)+")")
		easydraw.msg(" ")
		easydraw.msg("Press A to start the update anyway")
		easydraw.msg("Press B to cancel")
		easydraw.msg(" ")
		easydraw.msg("(Updating now will not harm your badge.)")
		title = "Update check"
		message = "An error occured while fetching information. You can still choose to start the OTA procedure."
else:
	easydraw.msg("No WiFi!")
	easydraw.msg_nosplit(str(version.build)+") "+version.name)
	easydraw.msg("Update anyway?")
	title = "Update check"
	message = "Could not connect to the WiFi network. You can still choose to start the OTA procedure."

ugfx.input_attach(ugfx.BTN_A, start)
try:
	ugfx.input_attach(ugfx.BTN_START, cancel)
except:
	pass
ugfx.input_attach(ugfx.BTN_B, cancel)

items = ["Cancel", "Start OTA update"]
callbacks = [system.home, system.ota]
callbacks[term.menu(title, items, 0, message)](True)
