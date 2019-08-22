import machine, display, time, system

#tz = machine.RTC().timezone()
#print("Default timezone:", tz)

display.drawFill(0xFFFFFF)
display.drawText(0,0,"Welcome to the BADGE.TEAM platform firmware!", 0x000000, "7x5")
display.drawText(0,6,"Please wait while we're setting things up...", 0x000000, "7x5")
display.flush()
time.sleep(2)

# Check if we have upgraded from a legacy firmware
legacy_mpr0 = machine.nvs_get_u16("badge", "mpr121.base.0")
if legacy_mpr0:
	display.drawFill(0xFFFFFF)
	display.drawText(0,0,"Welcome to the BADGE.TEAM platform firmware!", 0x000000, "7x5")
	display.drawText(0,6,"You have upgraded from an older firmware,", 0x000000, "7x5")
	display.drawText(0,12,"now migrating your settings...", 0x000000, "7x5")
	display.flush()
	legacy_mpr1 = machine.nvs_get_u16("badge", "mpr121.base.1")
	legacy_mpr2 = machine.nvs_get_u16("badge", "mpr121.base.2")
	legacy_mpr3 = machine.nvs_get_u16("badge", "mpr121.base.3")
	legacy_mpr4 = machine.nvs_get_u16("badge", "mpr121.base.4")
	legacy_mpr5 = machine.nvs_get_u16("badge", "mpr121.base.5")
	legacy_mpr6 = machine.nvs_get_u16("badge", "mpr121.base.6")
	legacy_mpr7 = machine.nvs_get_u16("badge", "mpr121.base.7")
	legacy_wifi_ssid = machine.nvs_getstr("badge", "wifi.ssid")
	legacy_wifi_password = machine.nvs_getstr("badge", "wifi.password")
	legacy_eink_type = machine.nvs_getstr("badge", "eink.dev.type")
	
	if legacy_eink_type:
		machine.nvs_setstr("system", "eink.dev.type", legacy_eink_type)
	
	machine.nvs_set_u16("system", "mpr121.base.0", legacy_mpr0)
	machine.nvs_set_u16("system", "mpr121.base.0", legacy_mpr1)
	machine.nvs_set_u16("system", "mpr121.base.0", legacy_mpr2)
	machine.nvs_set_u16("system", "mpr121.base.0", legacy_mpr3)
	machine.nvs_set_u16("system", "mpr121.base.0", legacy_mpr4)
	machine.nvs_set_u16("system", "mpr121.base.0", legacy_mpr5)
	machine.nvs_set_u16("system", "mpr121.base.0", legacy_mpr6)
	machine.nvs_set_u16("system", "mpr121.base.0", legacy_mpr7)
	machine.nvs_setstr("system", "wifi.ssid", legacy_wifi_ssid)
	machine.nvs_setstr("system", "wifi.password", legacy_wifi_password)
else:
	display.drawFill(0xFFFFFF)
	display.drawText(0,0,"Welcome to the BADGE.TEAM platform firmware!", 0x000000, "7x5")
	display.drawText(0,6,"This is a fresh install of the firmware", 0x000000, "7x5")
	display.drawText(0,12,"Now calibrating the touch buttons...", 0x000000, "7x5")
	display.flush()
	time.sleep(2)
	import _mpr121calib

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
machine.nvs_setint('system', 'factory_checked', 1)

display.drawFill(0xFFFFFF)
display.drawText(0,0,"Welcome to the BADGE.TEAM platform firmware!", 0x000000, "7x5")
display.drawText(0,6,"Setup completed!", 0x000000, "7x5")
display.drawText(0,12,"Restarting...", 0x000000, "7x5")

time.sleep(5)

system.home()
