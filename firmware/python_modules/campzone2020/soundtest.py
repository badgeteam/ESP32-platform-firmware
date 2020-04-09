import sndmixer, system, time, os, i2c, display
from machine import Pin, nvs_getint

on_selected = b'\xff\x00\xff'
on = b'\xf0\xf0\x00'
off = b'\x00\x00\x00'

fsize = 0
file_handle = None
playing = False

try:
    os.umountsd()
except:
    pass

try:
    os.mountsd()
    songs = [filename for filename in os.listfiles('sd', return_full=True) if filename.endswith('mp3')]
except:
    songs = []
songs.sort()

print(songs)

ledstate = [False] * 16
touchstate = [False] * 16

for index,_ in enumerate(songs):
    ledstate[index] = True

# Dummy flush
display.flush()
time.sleep(1)

# for i, state in enumerate(ledstate):
#     x = i % 4
#     y = int(i/4)
#     print(x, y, 0x00FFFF if state else 0x000000)
#     display.drawPixel(x, y, 0x00FFFF if state else 0x000000)

display.flush()
sndmixer.begin(1)

released = True

while True:
    try:
        buttons = bin(int.from_bytes(i2c.i2c_read_reg(25, 4, 2), 'little'))[2:]
        buttons = '0' * (8-len(buttons)) + buttons
        touchstate = [char == '1' for char in buttons]
        touchstate.reverse()

        if buttons == '0' * 8:
            released = True

        try:
            if (file_handle is not None and file_handle.tell() == fsize):
                if (file_handle is not None and file_handle.tell() == fsize):
                    file_handle.close()
        except:
            pass

        if True in touchstate and released:
            number = touchstate.index(True)
            print('playing %d' % number)
            vol = nvs_getint('system', 'volume') or 15
            fsize = os.stat(songs[number])[6]
            file_handle = open(songs[number], 'rb')
            mp3player = sndmixer.mp3_stream(file_handle)
            sndmixer.volume(mp3player, vol)
            sndmixer.play(mp3player)
            number = -1
            released = False

        time.sleep(0.01)
    except Exception as e:
        print(e)
        pass