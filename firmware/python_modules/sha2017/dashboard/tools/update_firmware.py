import tasks.otacheck as otacheck, easydraw, term, system, time, version, easywifi, badge, ugfx, orientation

orientation.default()

term.header(True, "Update check")
print("Checking for updates...")
print("")
print("Currently installed:",version.name,"(Build "+str(version.build)+")")

easydraw.msg("Current version:", "Update check", True)
easydraw.msg_nosplit(str(version.build)+") "+version.name)
time.sleep(3)
available = 0

badge.nvs_set_u8('badge','OTA.ready',0)

def start(pressed):
	if pressed:
		system.ota()

if not easywifi.status():
	if not easywifi.enable():
		easydraw.msg("Error: could not connect to WiFi!")

title = "Update check"
message = "?? Unknown state ??"
		
if easywifi.status():
	info = otacheck.download_info()
	if info:
			print("Server has: ",info['name']," (Build "+str(info['build'])+")")
			if info["build"] > version.build:
				badge.nvs_set_u8('badge','OTA.ready',0)
				print("Update available!")
				badge.nvs_set_u8('badge','OTA.ready',1)
				easydraw.msg("Update available!")
				easydraw.msg_nosplit(str(info["build"])+") "+info["name"])
				easydraw.msg("Update now?")
				title = "Firmware update available"
				message  = "A new firmware version is available. Update?\n"
				message += "Currently installed: "+version.name+" (Build "+str(version.build)+")\n"
				message += "Available          : "+info["name"]+" (Build "+str(info["build"])+")"
			elif info["build"] < version.build:
				badge.nvs_set_u8('badge','OTA.ready',0)
				print("Server has an older version.")
				badge.nvs_set_u8('badge','OTA.ready',1)
				easydraw.msg("Downgrade available!")
				easydraw.msg_nosplit(str(info["build"])+") "+info["name"])
				easydraw.msg("Downgrade now?")
				title = "Firmware downgrade available"
				message  = "An older firmware version is available. Update?\n"
				message += "Currently installed: "+version.name+" (Build "+str(version.build)+")\n"
				message += "Available          : "+info["name"]+" (Build "+str(info["build"])+")"
			else:
				print("You are up-to-date!")
				easydraw.msg("Up-to-date!")
				easydraw.msg_nosplit(str(version.build)+") "+version.name)
				easydraw.msg("Update anyway?")
				title = "Up-to-date"
				message = "You are up-to-date.\n"
				message += "Currently installed: "+version.name+" (Build "+str(version.build)+")\n"
				message += "Available          : "+info["name"]+" (Build "+str(info["build"])+")"
	else:
		print("An error occured!")
		easydraw.msg("Check failed.")
		easydraw.msg_nosplit(str(version.build)+") "+version.name)
		easydraw.msg("Update anyway?")
		title = "Update check"
		message = "An error occured while fetching information. You can still choose to start the OTA procedure."
else:
	easydraw.msg("No WiFi!")
	easydraw.msg_nosplit(str(version.build)+") "+version.name)
	easydraw.msg("Update anyway?")
	title = "Update check"
	message = "Could not connect to the WiFi network. You can still choose to start the OTA procedure."

ugfx.input_attach(ugfx.BTN_START, start)


items = ["Cancel", "Start OTA update"]
callbacks = [system.home, system.ota]
callbacks[term.menu(title, items, 0, message)](True)
