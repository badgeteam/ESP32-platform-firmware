import machine, display

def nvs_get_u8(space, item, default=None):
	res = machine.nvs_get_u8(space, item)
	if res == None:
		res = default
	return default

def nvs_set_u8(space, item, value):
	return machine.nvs_set_u8(space, item, value)

def nvs_get_u16(space, item, default=None):
	res = machine.nvs_get_u16(space, item)
	if res == None:
		res = default
	return default

def nvs_set_u16(space, item, value):
	return machine.nvs_set_u16(space, item, value)

def nvs_get_str(space, item, default=None):
	res = machine.nvs_getstr(space, item)
	if res == None:
		res = default
	return default

def nvs_set_str(space, item, value):
	return machine.nvs_setstr(space, item, value)

def leds_init():
	pass

def leds_enable():
	pass

def safe_mode():
	return False

def png_info(arg):
	return [0,0,0,0]

def setPower(arg):
	pass

deviceType = "FIXME"

def usb_volt_sense():
	return 0
