import machine, display, time, system

currentState = machine.nvs_getint('system', 'factory_checked')

display.drawFill(0xFFFFFF)
display.drawText(0,0,"Welcome to the BADGE.TEAM platform firmware!", 0x000000, "7x5")
display.drawText(0,6,"Please wait while we're setting things up...", 0x000000, "7x5")
display.flush()
time.sleep(2)

# Install icons to the filesystem if needed
try:
	media = uos.listdir("/media")
	icons = ["alert", "bell", "bug", "busy", "charge", "crown", "earth", "flag", "music", "ok", "wifi", "usb"]
	for icon in icons:
		if not icon+".png" in media:
			raise(BaseException(""))
except:
	import dashboard.resources.png_icons

# We have completed the factory script
machine.nvs_setint('system', 'factory_checked', 3)
system.home()
