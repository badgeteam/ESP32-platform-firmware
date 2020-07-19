import wifi, audio, time, system

if not wifi.status():
    audio.play('/cache/system/wifi_connecting.mp3')
    wifi.connect()
    wifi.wait()
    if not wifi.status():
        audio.play('/cache/system/wifi_failed.mp3')
        time.sleep(6)
        system.launcher()
    

url = 'https://icecast-bnr-cdp.triple-it.nl/bnr_mp3_128_03?.mp3'
audio.play(url)