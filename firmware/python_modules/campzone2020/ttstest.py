import wifi, ugTTS, os, time

try:
    os.mountsd()
except:
    pass

wifi.connect()
wifi.wait()
wifi.connect()

ugTTS.text_to_mp3('Whack-A-Mole', '/cache/testing.mp3')
ugTTS.text_to_mp3('Memeboard', '/cache/memeboard.mp3')
ugTTS.text_to_mp3('Sequencer', '/cache/sequencer.mp3')
ugTTS.text_to_mp3('Synthesizer', '/cache/synthesizer.mp3')