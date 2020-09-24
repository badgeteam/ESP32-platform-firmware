import machine, display, time, system

# Troopers 2020 "factory" tool
# Does function as factory tool but also implements an upgrade path from our old firmware

currentState = machine.nvs_getint('system', 'factory_checked') or 0

if currentState < 3:
	import dashboard.resources.png_icons as icons
	icons.install()

# We have completed the factory script
machine.nvs_setint('system', 'factory_checked', 3)
system.home()
