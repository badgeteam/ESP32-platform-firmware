import voltages as _voltages
_voltage   = _voltages.identification()
_names     = ["Techie", "Fixer", "Corporate", "Netrunner", "Rocker", "Prototype"]
_values    = [830, 1460, 2110, 2710, 3300, 9999]
_badgeType = len(_values)-1

# Determine badge type
for i in range(len(_values)):
	if _voltage < _values[i]:
		_badgeType = i
		break

def getName():
	return _names[_badgeType]

def getType():
	return _badgeType

def getVoltage():
	return _voltage
