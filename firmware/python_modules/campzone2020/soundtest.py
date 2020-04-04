import sndmixer, system, time, os, neopixel
from machine import Pin, nvs_getint

on_selected = b'\xff\x00\xff'
on = b'\xf0\xf0\x00'
off = b'\x00\x00\x00'

fsize = 0
file_handle = None
playing = False
try:
    os.mountsd()
    songs = [filename for filename in os.listfiles('sd', return_full=True) if filename.endswith('mp3')]
except:
    songs = []
songs.sort()

print(songs)

ledstate = [False] * 16
touchstate = [False] * 16
neopixel.enable()

for index,_ in enumerate(songs):
    ledstate[index] = True

# try:
# while True:
#     rows = [Pin(16, Pin.OUT), Pin(17, Pin.OUT), Pin(21, Pin.OUT), Pin(22, Pin.OUT)]
#     cols = [Pin(36, Pin.IN), Pin(39, Pin.IN), Pin(35, Pin.IN), Pin(34, Pin.IN)]
#     number = -1
#     for row_index, row in enumerate(rows):
#         row.value(1)
#         for col_index, col in enumerate(cols):
#             val = col.value()
#             touchstate[row_index * 4 + col_index] = val
#             if val:
#                 number = row_index * 4 + col_index
#         row.value(0)
#
#     if touchstate[-1]:
#         system.reboot()
#
#     if touchstate[-2] or (file_handle is not None and file_handle.tell() == fsize):
#         if (file_handle is not None and file_handle.tell() == fsize):
#             time.sleep(2)
#         print('%d of %d bytes' % (file_handle.tell(), fsize))
#         system.start('soundtest')
#
#     if number >= 0 and not playing:
#         print('playing %d' % number)
#         vol = nvs_getint('system', 'volume') or 15
#         fsize = os.stat(songs[number])[6]
#         file_handle = open(songs[number], 'rb')
#         sndmixer.begin(1)
#         mp3player = sndmixer.mp3_stream(file_handle)
#         sndmixer.volume(mp3player, vol)
#         sndmixer.play(mp3player)
#         number = -1
#         playing = True
#
#     neopixel.send(b''.join([on if value else off for value in ledstate]))
#
#     time.sleep(0.01)
# except Exception as e:
#     # Return to shell
#     print(e)
#     pass