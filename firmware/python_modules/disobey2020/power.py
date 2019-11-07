import mpr121 as _mpr121
from machine import Pin as _Pin

PIN_SDCARD = 19

_sdcard = _Pin(PIN_SDCARD, _Pin.OUT)

def leds(state):
	_mpr121.set(10, state)

def sdcard(state):
	_sdcard.value(state)
