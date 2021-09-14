# Based on https://github.com/pndurette/gTTS/

import urequests, json, binascii, audio, machine

_translate_url = 'https://translate.google.com/_/TranslateWebserverUi/data/batchexecute'
_GOOGLE_TTS_HEADERS = {
    "Referer": "http://translate.google.com/",
    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.106 Safari/537.36",
    "Content-Type": "application/x-www-form-urlencoded;charset=utf-8"
}
_ALWAYS_SAFE = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.-~'
_PREAMBLE = '"jQ1olc","[\\"'
_POSTAMBLE = '\\"]"'

def quote(text):
    return ''.join([c if c in _ALWAYS_SAFE else '%%%02X' % ord(c) for c in text])

def package_rpc(text, lang="en", slow=False):
    parameter = [text, lang, slow, "null"]
    escaped_parameter = json.dumps(parameter).replace(", ", ",").replace(": ", ":")

    rpc = [[["jQ1olc", escaped_parameter, None, "generic"]]]
    espaced_rpc = json.dumps(rpc).replace(", ", ",").replace(": ", ":")
    return "f.req={}&".format(quote(espaced_rpc))

def text_to_mp3(text, filename, lang='en'):
    text = text.replace(' ', '-')
    payload = package_rpc(text)

    request = urequests.post(_translate_url, data=payload,
                             headers=_GOOGLE_TTS_HEADERS)
    content = request.content.decode("ascii")
    preamble_pos = content.find(_PREAMBLE)
    if preamble_pos == -1:
        print("Got invalid response from Google Translate service: no preamble!")
        return False

    content = content[preamble_pos + len(_PREAMBLE):]

    postamble_pos = content.find(_POSTAMBLE)
    if postamble_pos == -1:
        print("Got invalid response from Google Translate service: no postamble!")
        return False

    content = content[:postamble_pos]
    data = binascii.a2b_base64(content)

    with open(filename, 'wb') as file:
        file.write(data)

    return True


def speak(text, filename='/cache/tts_temp.mp3', lang='en', volume=None):
    if volume == None:
        volume = machine.nvs_getint('system', 'volume')
    text_to_mp3(text, filename, lang)
    audio.play(filename, volume)
