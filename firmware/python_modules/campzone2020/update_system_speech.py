import wifi, ugTTS, os, audio, time, display, sys, system, machine

try:
    os.mountsd()
    os.mkdir('/cache/system')
except:
    pass

try:
    audio.play('/cache/system/updating_messages.mp3')
except BaseException as e:
    sys.print_exception(e)
    if not machine.nvs_get_u8("system", "ignore_crash"):
        print("Exception initialising system speech updater")
        system.crashedWarning()

wifi.connect()
wifi.wait()
wifi.connect()

if not wifi.status():
    audio.play('/cache/system/wifi_failed.mp3')
    time.sleep(3)
    system.launcher()

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
    ['Whack-A-Mole', '/cache/appnames/whack_a_mole.mp3'],
    ['Memeboard', '/cache/appnames/memeboard.mp3'],
    ['Sequencer', '/cache/appnames/sequencer.mp3'],
    ['Synthesizer', '/cache/appnames/synthesizer.mp3'],
    ['Hid Test', '/cache/appnames/hid_test.mp3'],
    ['Midi Test', '/cache/appnames/midi_test.mp3'],
]

try:
    display.drawFill(0x050505)
    display.flush()
except BaseException as e:
    sys.print_exception(e)
    if not machine.nvs_get_u8("system", "ignore_crash"):
        print("Exception setting system speech updater background")
        system.crashedWarning()

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

display.drawFill(0x007F00)
display.flush()
time.sleep(1)
system.launcher()