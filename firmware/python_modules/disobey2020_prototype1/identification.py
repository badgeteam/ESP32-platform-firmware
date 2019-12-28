_voltage   = 0
_names     = ["Techie", "Fixer", "Corporate", "Netrunner", "Rocker", "Prototype"]
_values    = [830, 1460, 2110, 2710, 3300, 9999]
_badgeType = len(_values)-1

# THIS IS PROTOTYPE, IGNORE VOLTAGE LEVEL

def getName():
	return _names[_badgeType]

def getType():
	return _badgeType

def getVoltage():
	return _voltage
