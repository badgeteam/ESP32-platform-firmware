import system, virtualtimers, display, keypad, sndmixer

LONG_PRESS_MS = const(1000)

# Application list
apps = [
    {'file': 'memeboard', 'title': 'Memeboard', 'titleAudio': '', 'colour': 0x00FF00, 'category': 'system'},
    {'file': 'sequencer', 'title': 'Sequencer', 'titleAudio': '', 'colour': 0xFF00FF, 'category': 'system'},
    {'file': 'touchtest', 'title': 'TouchTest', 'titleAudio': '', 'colour': 0x00FFFF, 'category': 'system'},
    {'file': 'hidtest', 'title': 'HIDTest', 'titleAudio': '', 'colour': 0xFF0000, 'category': 'system'},
    {'file': 'miditest', 'title': 'MIDITest', 'titleAudio': '', 'colour': 0xFF0000, 'category': 'system'},
]

presses = {}
page = 0

def get_app_range():
    begin = 16 * page
    end = begin + 16
    return begin, end

def get_app(key_index):
    app_index = key_index + (16 * page)
    if app_index >= 0 and app_index < len(apps):
        return apps[app_index]
    else:
        return None

def drawApps():
    begin, end = get_app_range()
    cur_page = apps[begin:end]
    display.drawFill(0x00)
    for index, app in enumerate(cur_page):
        x = index % 4
        y = int(index / 4)
        display.drawPixel(x, y, app['colour'])
    display.flush()

def start_app(key_index):
    app = get_app(key_index)
    print('got app', app)
    if app is not None:
        system.start(app['file'])

def play_app_audio(key_index):
    app = get_app(key_index)
    if app is not None:
        sndmixer.begin(1)  ## Might be started already, but that doesn't matter
        player = sndmixer.mp3(app['titleAudio'])
        sndmixer.volume(player, 255)
        sndmixer.play(player)

def on_long_press(key_index):
    global presses
    if key_index in presses:
        press = presses[key_index]
        press['is_long'] = True
        print('long press %d' % key_index)
    else:
        print('long press key not found')

    return -1

def on_key(key_index, pressed):
    global presses
    if pressed:
        press = {
            'is_long': False,
            'timer': lambda: on_long_press(key_index)
        }
        presses[key_index] = press
        virtualtimers.new(LONG_PRESS_MS, press['timer'], hfpm=True)
        print('press %d' %key_index)
    else:
        if key_index in presses:
            press = presses[key_index]
            if not press['is_long']:
                start_app(key_index)
            virtualtimers.delete(press['timer'])
            del presses[key_index]
        print('release %d' %key_index)

virtualtimers.activate(1000)
keypad.add_handler(on_key)
drawApps()