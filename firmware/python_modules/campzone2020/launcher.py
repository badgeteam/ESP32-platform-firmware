import system, virtualtimers, display, keypad, easyaudio

LONG_PRESS_MS = const(1000)

# Application list
apps = [
    {'slug': 'memeboard', 'title': 'Memeboard', 'colour': 0xFE3010, 'category': 'system'},
    {'slug': 'sequencer', 'title': 'Sequencer', 'colour': 0xE35722, 'category': 'system'},
    {'slug': 'synthesizer', 'title': 'Synthesizer', 'colour': 0xFCCF0D, 'category': 'system'},
    {'slug': 'whackamole', 'title': 'Whack-A-Mole', 'colour': 0x2721EC, 'category': 'system'},
    {'slug': 'simonsays', 'title': 'Simon Says', 'colour': 0x441C7A, 'category': 'system'},
    {'slug': 'hidtest', 'title': 'HIDTest', 'colour': 0x29BF12, 'category': 'system'},
    {'slug': 'miditest', 'title': 'MIDITest', 'colour': 0x17BEBB, 'category': 'system'},
    {'slug': 'ttstest', 'title': 'TTS Test', 'colour': 0x6F1152, 'category': 'system'},
    {'slug': 'ttstest', 'title': 'Simon Says', 'colour': 0xFF619B, 'category': 'system'},
    {'slug': 'ttstest', 'title': 'Simon Says', 'colour': 0xFFF1D0, 'category': 'system'},
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
        system.start(app['slug'])

def play_app_audio(key_index):
    app = get_app(key_index)
    if app is not None:
        easyaudio.play('/cache/%s.mp3' % app['slug'])

def on_long_press(key_index):
    global presses
    if key_index in presses:
        press = presses[key_index]
        press['is_long'] = True
        print('long press %d' % key_index)
        play_app_audio(key_index)
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

virtualtimers.activate(100)
keypad.add_handler(on_key)
drawApps()