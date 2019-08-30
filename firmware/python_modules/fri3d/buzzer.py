from machine import Pin, PWM, random
from rtttl import RTTTL
from _buzzer_mapping import buzzer as pin_num
import time

__pwm = PWM(Pin(pin_num, Pin.OUT), duty=0)
__pwm.duty(0)

def freq(freq=None):
	if freq:
		__pwm.freq(freq)
		__pwm.duty(50)
	else:
		__pwm.duty(0)

def tone(freq, duration, pause=None):
	global pwm
	freq = round(freq)
	if not pause:
		duration = round(duration * 0.9)
		pause = round(duration * 0.1)
	if freq < 1:
		__pwm.duty(0)
	else:
		__pwm.freq(freq)
		__pwm.duty(50)
		time.sleep_ms(duration)
		__pwm.duty(0)
		time.sleep_ms(pause)

def play(song):
	tune = RTTTL(song)
	for freq, duration in tune.notes():
		tone(freq, duration)

NOTES = dict(
 c=261,
 d=294,
 e=329,
 f=349,
 g=391,
 gS=415,
 a=440,
 aS=455,
 b=466,
 cH=523,
 cSH=55,
 dH=587,
 dSH=62,
 eH=659,
 fH=698,
 fSH=74,
 gH=784,
 gSH=83,
 aH=880 )
