import system, virtualtimers, display, keypad, touchpads, audio, valuestore, wifi
import os, ugTTS, term_menu, time, machine

LONG_PRESS_MS = const(1000)

wifi.connect()

def download_tts(app):
    tts_text = app['name']
    print("Preparing TTS: ", tts_text)
    try:
        ugTTS.text_to_mp3(tts_text, '/cache/appnames/%s.mp3' % app['slug'])
        print("TTS: Success")
        return True
    except:
        return False


def update_mp3_cache():
    global apps
    # freediskpercent = int(os.statvfs('/cache')[3] / (os.statvfs('/cache')[2] / 100))
    freediskpercent = 100  # Above free disk space check doesn't work
    if freediskpercent > 25: # do not fill filesystem when free space < 25%
        if not wifi.status():
            wifi.connect()
            return 2000
        mp3files = os.listdir('/cache/appnames')
        for app_index in apps:
            try:
                app = apps[app_index]
                mp3file = '%s.mp3' % app['slug']
                if mp3file not in mp3files:
                    try:
                        audio.play('/cache/system/generating_app_name.mp3')
                    except:
                        pass
                    download_tts(app)
                    try:
                        audio.play('/cache/appnames/%s' % mp3file)
                    except:
                        pass
            except:
                pass
    else:
        print("Flash almost full, skipping text-to-speech")
    return 2000

# Application list
apps = {}
app_list_last_modified = 0
def update():
    global apps
    global app_list_last_modified
    new_modified = valuestore.last_modified('system', 'launcher_items')
    if new_modified != app_list_last_modified:
        app_list_last_modified = new_modified
        apps = valuestore.load('system', 'launcher_items') or {"0":{"slug":"norbert","name":"Synthesizer","colour":"#4A90E2"}}
        drawApps()
    return 500

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
        audio.play('/cache/appnames/%s.mp3' % app['slug'])

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


def on_left(pressed):
    global page
    if pressed:
        page = 0 if page <= 0 else (page-1)
        drawApps()
        print('page %d' % page)


def on_right(pressed):
    global page
    if pressed:
        page += 1
        drawApps()
        print('page %d' % page)


touchpads.on(touchpads.LEFT, on_left)
touchpads.on(touchpads.RIGHT, on_right)

if not machine.nvs_getint('system', 'welcome_done'):
    machine.nvs_setint('system', 'welcome_done', 1)
    audio.play('/cache/system/welcome.mp3')
    time.sleep(3)
    audio.play('/cache/system/press.mp3')
    time.sleep(3)

virtualtimers.activate(500)
keypad.add_handler(on_key)
update()
virtualtimers.new(500, update)  # Refresh app list twice per second
virtualtimers.new(2000, update_mp3_cache)  # Refresh app names every 2 seconds

## Launch terminal menu
# menu = term_menu.UartMenu()
# menu.main()