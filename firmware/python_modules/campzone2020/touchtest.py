import system, neopixel, time
import sndmixer
from machine import Pin, nvs_getint

vol = nvs_getint('system', 'volume') or 15
on = b'\xff\x00\xff'
# on = b'\x00\xf0\xf0'
off = b'\x00\x00\x00'

bck, ws, data = Pin(4, Pin.OUT), Pin(15, Pin.OUT), Pin(33, Pin.OUT)
data.value(0)

rows = [Pin(16, Pin.OUT), Pin(17, Pin.OUT), Pin(21, Pin.OUT), Pin(22, Pin.OUT)]
cols = [Pin(36, Pin.IN), Pin(39, Pin.IN), Pin(35, Pin.IN), Pin(34, Pin.IN)]

touchstate = [0] * 16
neopixel.enable()

sndmixer.begin(1)
synth = sndmixer.synth()
sndmixer.volume(synth, vol)
sndmixer.waveform(synth, 0)

try:
    while True:
        rows = [Pin(16, Pin.OUT), Pin(17, Pin.OUT), Pin(21, Pin.OUT), Pin(22, Pin.OUT)]
        cols = [Pin(36, Pin.IN), Pin(39, Pin.IN), Pin(35, Pin.IN), Pin(34, Pin.IN)]
        frequency = 0
        for row_index, row in enumerate(rows):
            row.value(1)
            for col_index, col in enumerate(cols):
                val = col.value()
                touchstate[row_index * 4 + col_index] = val
                if val:
                    number = row_index * 4 + col_index
                    frequency = 440 * (2**(number/12))
            row.value(0)

        if frequency:
            sndmixer.volume(synth, vol)
            sndmixer.freq(synth, int(frequency))
        else:
            sndmixer.volume(synth, 0)
        neopixel.send(b''.join([on if value else off for value in touchstate]))

        if(touchstate[-1] and touchstate[-5]):
            system.start('soundtest')
        elif(touchstate[-4] and touchstate[-8]):
            system.start('rainbow')

        time.sleep(0.01)
except:
    # Return to shell
    pass