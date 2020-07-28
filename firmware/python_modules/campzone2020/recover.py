import wifi, ugTTS, os, audio, time, display, sys, system, machine

try:
    os.mountsd()
except:
    pass

print('=== Starting recovery procedure ===')

try:
    display.drawFill(0x252525)
    display.flush()
except BaseException as e:
    sys.print_exception(e)
    if not machine.nvs_get_u8("system", "ignore_crash"):
        print("Exception setting system speech updater background")
        system.crashedWarning()

print('Connecting to WiFi')
wifi.connect()
wifi.wait()
wifi.connect()

if not wifi.status():
    print('Failed to connect to WiFi. Please configure your badge\'s WiFi from your web browser.')
    system.launcher()

try:
    ugTTS.speak('Initialising files on your badge')
    time.sleep(3)
except:
    pass

pairs = [
    ['Updating system speech messages.', '/cache/system/updating_messages.mp3'],
    ['Welcome to your CampZone badge.', '/cache/system/welcome.mp3'],
    ['Start apps with a short press. Hear their name with a long press.', '/cache/system/press.mp3'],
    ['Connecting to WiFi.', '/cache/system/wifi_connecting.mp3'],
    ['WiFi connected.', '/cache/system/wifi_connected.mp3'],
    ['Failed to connect to WiFi. Please check your settings in the browser.', '/cache/system/wifi_failed.mp3'],
    ['Checking for system updates.', '/cache/system/checking_system_update.mp3'],
    ['System update available.', '/cache/system/system_update_available.mp3'],
    ['No updates available.', '/cache/system/no_update_available.mp3'],
    ['Press accept to continue, or decline to cancel.', '/cache/system/accept_decline.mp3'],
    ['Generating a spoken name for app.', '/cache/system/generating_app_name.mp3'],
    ['Downloading necessary files.', '/cache/system/downloading_files.mp3'],
    ['Check for system updates.', '/cache/system/check_system_update.mp3'],
    ['Works.', '/cache/system/works.mp3'],
]

for index, pair in enumerate(pairs):
    progress = index / len(pairs)
    for i in range(int(progress * 16)):
        x = i % 4
        y = int(i / 4)
        display.drawPixel(x, y, 0x007F00)
    display.flush()
    try:
        text, path = pair
        print('Generating speech: "%s"' % text)
        ugTTS.text_to_mp3(text, filename=path)
        time.sleep(0.1)
    except BaseException as e:
        sys.print_exception(e)
        if not machine.nvs_get_u8("system", "ignore_crash"):
            print("Exception updating file %s" % path)
            system.crashedWarning()

try:
    display.drawFill(0x007F00)
    display.flush()
except:
    pass

import woezel
apps_json = {"0":{"slug":"synthesizer","name":"Synthesizer","colour":"#FF7F00"},"1":{"slug":"fourinarow","name":"FourInARow","colour":"#F8B700"},"2":{"slug":"bnr_radio","name":"BNR Radio","colour":"#4A90E2"},"3":{"slug":"midi_controller","name":"MIDI Controller","colour":"#7ED321"},"4":{"slug":"cybertyper","name":"CyberTyper","colour":"#D0021B"}}
for index in range(len(list(apps_json.keys()))):
    try:
        app = apps_json[str(index)]
        progress = index / len(list(apps_json.keys()))
        for i in range(int(progress * 16)):
            x = i % 4
            y = int(i / 4)
            display.drawPixel(x, y, 0x00007F)
        display.flush()
        print('Installing %s' % app['slug'])
        woezel.install(app['slug'])
    except:
        pass

try:
    display.drawFill(0x00007F)
    display.flush()
except:
    pass

try:
    import valuestore
    valuestore.save('system', 'launcher_items', apps_json)
except:
    pass

system.launcher()