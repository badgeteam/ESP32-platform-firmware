import sndmixer, system, time, os, i2c, display, gc
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
    songs = [filename for filename in os.listfiles('sd/Samples', return_full=True) if
             '/._' not in filename and
             (filename.endswith('wav'))]
except:
    songs = []
songs.sort()

print(songs)

ledstate = [False] * 16
touchstate = [False] * 16

for index,_ in enumerate(songs):
    if index >= len(ledstate):
        break
    ledstate[index] = True

# Dummy flush
display.flush()
time.sleep(1)

for i, state in enumerate(ledstate):
    x = i % 4
    y = int(i/4)
    display.drawPixel(x, y, 0x00FFFF if state else 0x000000)

display.flush()
sndmixer.begin(4, True)
sndmixer.beat_sync_start(120)
file_handles = {}

released = True

while True:
    # try:
    gc.collect()
    buttons = bin(int.from_bytes(i2c.i2c_read_reg(25, 4, 2), 'little'))[2:]
    buttons = '0' * (8-len(buttons)) + buttons
    touchstate = [char == '1' for char in buttons]
    touchstate.reverse()

    if buttons == '0' * 8:
        released = True

    if True in touchstate and released:
        released = False
        number = touchstate.index(True)
        print('playing %d' % number)
        vol = nvs_getint('system', 'volume') or 15
        file_handle = open(songs[number], 'rb')
        player = sndmixer.wav_stream(file_handle)
        print('got channel id %d' % player)
        if player is None:
            continue
        file_handles[player] = file_handle
        sndmixer.on_finished(player, lambda _: file_handles[player].close())
        sndmixer.volume(player, vol)
        sndmixer.loop(player, True)
        sndmixer.start_at_next(player, 4)
        number = -1

    time.sleep(0.01)
    # except Exception as e:
    #     print(e)
    #     pass