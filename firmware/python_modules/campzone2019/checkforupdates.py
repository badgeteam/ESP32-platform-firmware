import tasks.otacheck as otacheck, term, system, time, version, wifi, machine

term.header(True, "Update check")
print("Checking for updates...")
print("")
print("Currently installed:",version.name,"(Build "+str(version.build)+")")

print("Current version:", "Update check", True)
print(str(version.build)+") "+version.name)
available = 0

machine.nvs_setint('badge','OTA.ready', 0)

def start(pressed):
	if pressed:
		system.ota()

if not wifi.status():
	wifi.connect()
	if not wifi.wait():
		print("Error: could not connect to WiFi!")

title = "Update check"
message = "?? Unknown state ??"
		
if wifi.status():
	info = otacheck.download_info()
	if info:
			print("Server has: ",info['name']," (Build "+str(info['build'])+")")
			if info["build"] > version.build:
				machine.nvs_setint('badge','OTA.ready',0)
				print("Update available!")
				machine.nvs_setint('badge','OTA.ready',1)
				print("Update available!")
				print(str(info["build"])+") "+info["name"])
				print("Update now?")
				title = "Firmware update available"
				message  = "A new firmware version is available. Update?\n"
				message += "Currently installed: "+version.name+" (Build "+str(version.build)+")\n"
				message += "Available          : "+info["name"]+" (Build "+str(info["build"])+")"
			elif info["build"] < version.build:
				machine.nvs_setint('badge','OTA.ready',0)
				print("Server has an older version.")
				machine.nvs_setint('badge','OTA.ready',1)
				print("Downgrade available!")
				print(str(info["build"])+") "+info["name"])
				print("Downgrade now?")
				title = "Firmware downgrade available"
				message  = "An older firmware version is available. Update?\n"
				message += "Currently installed: "+version.name+" (Build "+str(version.build)+")\n"
				message += "Available          : "+info["name"]+" (Build "+str(info["build"])+")"
			else:
				print("You are up-to-date!")
				print("Up-to-date!")
				print(str(version.build)+") "+version.name)
				print("Update anyway?")
				title = "Up-to-date"
				message = "You are up-to-date.\n"
				message += "Currently installed: "+version.name+" (Build "+str(version.build)+")\n"
				message += "Available          : "+info["name"]+" (Build "+str(info["build"])+")"
	else:
		print("An error occured!")
		print("Check failed.")
		print(str(version.build)+") "+version.name)
		print("Update anyway?")
		title = "Update check"
		message = "An error occured while fetching information. You can still choose to start the OTA procedure."
else:
	print("No WiFi!")
	print(str(version.build)+") "+version.name)
	print("Update anyway?")
	title = "Update check"
	message = "Could not connect to the WiFi network. You can still choose to start the OTA procedure."

# ugfx.input_attach(ugfx.BTN_START, start)

items = ["Cancel", "Start OTA update"]
callbacks = [system.home, system.ota]
callbacks[term.menu(title, items, 0, message)]()
