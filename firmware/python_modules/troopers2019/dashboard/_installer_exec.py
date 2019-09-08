import wifi, woezel, gc, time, system

def showMessage(msg, error=False, icon_wifi=False, icon_ok=False):
	import term, easydraw
	term.header(True, "Installer")
	print(msg)
	if error:
		easydraw.messageCentered("ERROR\n\n"+msg, True, "/media/alert.png")
	elif icon_wifi:
		easydraw.messageCentered("PLEASE WAIT\n\n"+msg, True, "/media/wifi.png")
	elif icon_ok:
		easydraw.messageCentered(msg, True, "/media/ok.png")
	else:
		easydraw.messageCentered("PLEASE WAIT\n\n"+msg, True, "/media/busy.png")

wifi.connect()

if not wifi.wait():
	showMessage("Failed to connect to WiFi!", True, False, False)
	time.sleep(2)
	system.start("dashboard.installer")

with open("/cache/installList") as f:
	apps = f.read()

for app in apps.split("\n"):
	print("Now installing app", app)
	try:
		gc.collect()
		woezel.install(app)
	except woezel.LatestInstalledError:
		pass
	except BaseException as e:
		showMessage("Failed to install "+app+"!", True)
		print("WOEZEL ERROR", e)
		time.sleep(2)
		system.start("dashboard.installer")

with open("/cache/installList", "w") as f:
	f.write("")

showMessage("OK\n\n"+"App installation completed.", False, False, True)
time.sleep(2)
system.start("dashboard.installer")
