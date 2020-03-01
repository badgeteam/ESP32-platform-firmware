import machine

PIN_VBAT   = 39
PIN_VUSB   = 36
PIN_VIDENT = 32

_vbat = machine.ADC(PIN_VBAT)
_vbat.atten(machine.ADC.ATTN_11DB)

_vusb = machine.ADC(PIN_VUSB)
_vusb.atten(machine.ADC.ATTN_11DB)

_vident = machine.ADC(PIN_VIDENT)
_vident.atten(machine.ADC.ATTN_11DB)
_vident.width(machine.ADC.WIDTH_12BIT)

def usb():
	'''
	Read USB input voltage
	:return: integer, voltage in mV
	'''
	return int(_vusb.read()*1.97)

def battery():
	'''
	Read battery voltage
	:return: integer, voltage in mV
	'''
	return int(_vbat.read()*1.94)

def identification():
	'''
	Read identification voltage
	:return: integer, voltage in mV
	'''
	return _vident.read()
