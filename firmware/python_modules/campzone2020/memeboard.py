import audio, os, display, keypad

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
    songs = [filename for filename in os.listfiles('sd', return_full=True) if
             '/._' not in filename and
             (filename.endswith('mp3'))]
except:
    songs = []
songs.sort()

print(songs)

ledstate = [False] * 16

for index,_ in enumerate(songs):
    if index >= len(ledstate):
        break
    ledstate[index] = True

for i, state in enumerate(ledstate):
    x = i % 4
    y = int(i/4)
    display.drawPixel(x, y, 0x00FFFF if state else 0x000000)

display.flush()

def on_key(key_index, pressed):
    if not pressed:
        return
    song = songs[key_index]
    audio.play(song)

keypad.add_handler(on_key)