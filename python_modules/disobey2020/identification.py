import voltages as _voltages
_voltage    = _voltages.identification()
_names      = ["Techie", "Fixer", "Corporate", "Netrunner", "Rocker"]
_badgeTypes = {
    2000: {"type": b"t", "name":"Techie"},
    5100: {"type": b"f", "name":"Fixer"},
    12000: {"type": b"c", "name":"Corporate"},
    27000: {"type": b"n", "name":"Netrunner"},
    100000: {"type": b"r", "name":"Rocker"}
}
_voltages    = [830, 1460, 2110, 2710, 3300, 9999]

_r1 = 10000

_adcRef = 3.9
_v_div = 3.3
_adcResolution = 4096


def getVoltage():
	return _voltage

def takeClosest(myList, myNumber):
    """
    Assumes myList is sorted. Returns closest value to myNumber.

    If two numbers are equally close, return the smallest number.
    """
    pos = bisect_left(myList, myNumber)
    if pos == 0:
        return myList[0]
    if pos == len(myList):
        return myList[-1]
    before = myList[pos - 1]
    after = myList[pos]
    if after - myNumber < myNumber - before:
        return after
    else:
        return before

def measure_resistor():
    """
    Reads ADC value on type-detection pin

    Returns measured resistance as integer
    """
    meas = getVoltage()
    adc_voltage = (meas / _adcResolution) * _adcRef
    r2 = int((_r1 * adc_voltage) / (_v_div - adc_voltage))
    return r2

def detect_type():
    """
    Takes measured resistance and matches it to key LUT

    Returns badge type from LUT.
    If detection fails, returns False
    """
    res = measure_resistor()
    if res > 0 and res < 1000000:
        closest_match = min(_badgeTypes, key=lambda x:abs(x - res))
        return _badgeTypes[closest_match]
    else:
    	return False

def getName():
	return detect_type()["name"]

def getType():
	return detect_type()["type"]
