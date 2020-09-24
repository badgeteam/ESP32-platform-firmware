import machine, display, consts, time, samd

deviceType = consts.INFO_HARDWARE_NAME

def init():
	pass

def eink_init():
	pass

def eink_png(x,y,img):
	display.drawPng(x,y,img)

def nvs_erase_key(space, item):
	return machine.nvs_erase(space, item)

def nvs_get_u8(space, item, default=None):
	if space == "badge":
		space = "system"
	res = machine.nvs_get_u8(space, item)
	if res == None:
		res = default
	return res

def nvs_set_u8(space, item, value):
	if space == "badge":
		space = "system"
	return machine.nvs_set_u8(space, item, value)

def nvs_get_u16(space, item, default=None):
	if space == "badge":
		space = "system"
	res = machine.nvs_get_u16(space, item)
	if res == None:
		res = default
	return res

def nvs_set_u16(space, item, value):
	if space == "badge":
		space = "system"
	return machine.nvs_set_u16(space, item, value)

def nvs_get_str(space, item, default=None):
	if space == "badge":
		space = "system"
	res = machine.nvs_getstr(space, item)
	if res == None:
		res = default
	return res

def nvs_set_str(space, item, value):
	if space == "badge":
		space = "system"
	return machine.nvs_setstr(space, item, value)

def led(l, r, g, b):
	samd.led(l, r, g, b)

def safe_mode():
	return False # We still have a recovery mode, just not this "safe" mode anymore

def png_info(arg):
	return display.pngInfo(arg)

def png(x,y,arg):
	return display.drawPng(x,y,arg)

def setPower(state):
	pass

def usb_volt_sense():
	return samd.read_usb()

def battery_volt_sense():
	return samd.read_battery()

def eink_busy_wait():
	pass

def eink_busy():
	return False

def vibrator_init():
	pass

def vibrator_activate(duration):
	pass
