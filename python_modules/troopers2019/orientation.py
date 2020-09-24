import version, machine, display, buttons

def getDefault():
	res = machine.nvs_get_u16('system', 'orientation')
	if not res:
		res = version.default_orientation
	return res

def setDefault(value):
	if value == 0 or value == 90 or value == 180 or value == 270:
		machine.nvs_set_u16('system', 'orientation', value)
		return True
	return False

#---

def default(rotateButtons=True):
	value = getDefault()
	if rotateButtons:
		buttons.rotate(value)
	display.orientation(value)

def landscape(rotateButtons=True):
	if rotateButtons:
		buttons.rotate(0)
	display.orientation(0)

def portrait(rotateButtons=True):
	if rotateButtons:
		buttons.rotate(90)
	display.orientation(90)

#---

def isLandscape(value=None):
	if value == None:
		value = display.orientation()
	if value==90 or value==270:
		return False
	return True

def isPortrait(value=getDefault()):
	if value==90 or value==270:
		return True
	return False
