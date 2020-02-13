# Badge variant identification for the Disobey 2020 badge
import voltages as _voltages

# Types
_badge_types = {
    2000: {"type": b"t", "name":"Techie"},
    5100: {"type": b"f", "name":"Fixer"},
    12000: {"type": b"c", "name":"Corporate"},
    27000: {"type": b"n", "name":"Netrunner"},
    100000: {"type": b"r", "name":"Rocker"}
}

# Determine badge type
_voltage = _voltages.identification()
_r1 = 10000
_v_div = 3300
_r2 = int((_r1 * _voltage) / (_v_div - _voltage))

if _r2 > 0 and _r2 < 1000000:
    _closest_match = min(_badge_types, key=lambda x:abs(x - _r2))
    _badge_type = _badge_types[_closest_match]
else:
    _badge_type = {"type": b"u", "name":"Unknown"}

# Public functions
def getName():
    '''
    Get the human readable name of the current badge type
    :return: string, full type name
    '''
    return _badge_type["name"]

def getType():
    '''
    Get the machine readable identifier of the current badge type
    :return: bytestring, identifier
    '''
    return _badge_type["type"]
