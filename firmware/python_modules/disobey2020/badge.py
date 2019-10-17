import machine, display, mpr121, consts, time, neopixel

deviceType = consts.INFO_HARDWARE_NAME

_vbat = machine.ADC(39)
_vbat.atten(machine.ADC.ATTN_11DB)
_vusb = machine.ADC(36)
_vusb.atten(machine.ADC.ATTN_11DB)

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

def leds_init():
	pass

def leds_enable():
	neopixel.enable()

def leds_send_data(data, length=0):
	neopixel.send(data)

def safe_mode():
	return False # We still have a recovery mode, just not this "safe" mode anymore

def png_info(arg):
	return display.pngInfo(arg)

def png(x,y,arg):
	return display.drawPng(x,y,arg)

def setPower(state):
	mpr121.set(10, state)

def usb_volt_sense():
	return int(_vusb.read()*1.97) # Determined by measuring the relevant voltage using a shitty multimeter :-)

def battery_volt_sense():
	return int(_vbat.read()*1.94) # Determined by measuring the relevant voltage using a shitty multimeter :-)

def eink_busy_wait():
	pass

def eink_busy():
	return False

def vibrator_init():
	pass

def vibrator_activate(duration):
	pass
