import system, display, keypad, machine
import sndmixer

vol = 255#machine.nvs_getint('system', 'volume') or 15
on = 0xFF00FF
off = 0x000000

sndmixer.begin(16)
channels = [None] * 16

def on_key(key_index, pressed):
    x, y = key_index % 4, int(key_index / 4)
    if pressed:
        frequency = int(440 * (2**(key_index/12)))
        synth = sndmixer.synth()
        channels[key_index] = synth
        sndmixer.volume(synth, vol)
        sndmixer.waveform(synth, frequency)
        sndmixer.play(synth)
        display.drawPixel(x, y, on)
        display.flush()
    else:
        if channels[key_index] is not None:
            sndmixer.stop(channels[key_index])
            display.drawPixel(x, y, 0x00)
            display.flush()

keypad.add_handler(on_key)