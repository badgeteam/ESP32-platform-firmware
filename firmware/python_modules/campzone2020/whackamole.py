from random import randint
import keypad, display, virtualtimers, audio

# https://freesound.org/people/petenice/sounds/9509/
# https://freesound.org/people/Ekokubza123/sounds/104183/


moles = []
difficulty = 1.0
score = 0

hit_audio = '/sd/Whack-A-Mole/104183__ekokubza123__punch.wav'
miss_audio = '/sd/Whack-A-Mole/9509__petenice__whoosh.wav'


def has_mole(x, y):
    for (_x,_y) in moles:
        if _x == x and _y == y:
            return True
    return False


def remove_mole(x, y):
    global moles
    for index, (_x,_y) in enumerate(moles):
        if _x == x and _y == y:
            del moles[index]


def on_key(index, pressed):
    global difficulty
    if not pressed:
        return
    x, y = keypad.index_to_coords(index)
    if has_mole(x, y):
        remove_mole(x, y)
        audio = hit_audio
        difficulty += 0.3
    else:
        audio = miss_audio
    audio.play(audio)
    draw()


def draw():
    display.drawFill(0x00)
    for (x,y) in moles:
        display.drawPixel(x, y, 0xB27300)
    display.flush()

def update():
    global moles
    if len(moles) == 0:
        no_moles = int(difficulty)
        for _ in range(no_moles):
            x, y = randint(0, 3), randint(0, 3)
            moles.append((x,y))
        draw()
        return 2000 - (difficulty * 400)
    else:
        moles = []
        draw()
        return 2000 - (difficulty * 300)

virtualtimers.begin(1000)
virtualtimers.new(0, update, False)
keypad.add_handler(on_key)