import machine, display, mpr121, eink

def init():
	pass

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

def leds_init():
	pass

def leds_enable():
	mpr121.set(10, true);

def safe_mode():
	return False

def png_info(arg):
	return display.png_info(arg)

def png(x,y,arg):
	return display.png(x,y,arg)

def setPower(state):
	mpr121.set(10, state);

deviceType = "FIXME"

def usb_volt_sense():
	return 0

def battery_volt_sense():
	return 0

def eink_busy_wait():
	eink.busy_wait()
