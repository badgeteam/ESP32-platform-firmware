# Analog voltage readout for the Disobey 2020 badge
import machine

# Pins
PIN_VBAT   = 39
PIN_VUSB   = 36
PIN_VIDENT = 32

# ADCs
_vbat = machine.ADC(PIN_VBAT)
_vbat.atten(machine.ADC.ATTN_11DB)
_vbat.width(machine.ADC.WIDTH_12BIT)

_vusb = machine.ADC(PIN_VUSB)
_vusb.atten(machine.ADC.ATTN_11DB)
_vusb.width(machine.ADC.WIDTH_12BIT)

_vident = machine.ADC(PIN_VIDENT)
_vident.atten(machine.ADC.ATTN_11DB)
_vident.width(machine.ADC.WIDTH_12BIT)

# Public functions
def usb():
    '''
    Read USB input voltage
    :return: integer, voltage in mV
    '''
    return int((_vusb.read() / 4096) * 3.9 * 2000)
    # Note: physically connected through a 100k/100k resistor divider

def battery():
    '''
    Read battery voltage
    :return: integer, voltage in mV
    '''
    return int((_vbat.read() / 4096) * 3.9 * 2000)
    # Note: physically connected through a 100k/100k resistor divider

def identification():
    '''
    Read identification voltage
    :return: integer, voltage in mV
    '''
    return int((_vident.read() / 4096) * 3.9 * 1000)
