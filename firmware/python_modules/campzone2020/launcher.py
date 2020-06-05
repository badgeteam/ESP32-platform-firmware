import system, virtualtimers, display, keypad, easyaudio, valuestore

LONG_PRESS_MS = const(1000)

# Application list
apps = valuestore.load('system', 'launcher_items') or {}

presses = {}
page = 0

def get_app(key_index):
    app_index = str(key_index + (16 * page))
    if app_index in apps:
        return apps[app_index]
    else:
        return None

def drawApps():
    display.drawFill(0x00)
    for index in range(0, 16):
        app = get_app(index)
        if app:
            x = index % 4
            y = int(index / 4)
            colour = int(app['colour'].replace("#","0x"),16)
            display.drawPixel(x, y, colour)
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