# Based on https://github.com/Boudewijn26/gTTS-token and https://github.com/pndurette/gTTS/

import urequests, re, math, time

_translate_url = 'https://translate.google.com/'
_token_key = None
_SALT_1 = "+-a^+6"
_SALT_2 = "+-3^+b+-f"
_GOOGLE_TTS_HEADERS = {
    "Referer": "http://translate.google.com/",
    "User-Agent":
        "Mozilla/5.0 (Windows NT 10.0; WOW64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/47.0.2526.106 Safari/537.36"
}

def _get_token_key():
    response = urequests.get("https://translate.google.com/")
    tkk_expr = re.search("(tkk:.*?),", response.text)
    if not tkk_expr:
        raise ValueError(
            "Unable to find token seed! Did https://translate.google.com change?"
        )

    tkk_expr = tkk_expr.group(1)
    try:
        # Grab the token directly if available
        token = re.search("[0-9]+\.[0-9]+", tkk_expr).group(0)
    except AttributeError:
        # Generate the token using parts present in token key
        timestamp = time.time()
        hours = int(math.floor(timestamp / 3600))
        a = re.search("a\\\\x3d(-?\d+);", tkk_expr).group(1)
        b = re.search("b\\\\x3d(-?\d+);", tkk_expr).group(1)

        token = str(hours) + "." + str(int(a) + int(b))

    return token

""" Functions used by the token calculation algorithm """
def _rshift(val, n):
    return val >> n if val >= 0 else (val + 0x100000000) >> n

def _work_token(a, seed):
    for i in range(0, len(seed) - 2, 3):
        char = seed[i + 2]
        d = ord(char[0]) - 87 if char >= "a" else int(char)
        d = _rshift(a, d) if seed[i + 1] == "+" else a << d
        a = a + d & 4294967295 if seed[i] == "+" else a ^ d
    return a

def _get_crypto_token(text):
    global _token_key
    if _token_key is None:
        _token_key = _get_token_key()

    [first_seed, second_seed] = _token_key.split(".")

    try:
        d = bytearray(text.encode('UTF-8'))
    except UnicodeDecodeError:
        # This will probably only occur when d is actually a str containing UTF-8 chars, which means we don't need
        # to encode.
        d = bytearray(text)

    a = int(first_seed)
    for value in d:
        a += value
        a = _work_token(a, _SALT_1)
    a = _work_token(a, _SALT_2)
    a ^= int(second_seed)
    if 0 > a:
        a = (a & 2147483647) + 2147483648
    a %= 1E6
    a = int(a)
    return str(a) + "." + str(a ^ int(first_seed))

def text_to_mp3(text, filename, lang='en'):
    text = text.replace(' ', '-')
    crypto_token = _get_crypto_token(text)
    params = {'ie': 'UTF-8',
               'q': text,
               'tl': lang,
               # 'ttsspeed': 1,
               'total': 1,
               'idx': 0,
               'client': 'tw-ob',
               'textlen': len(text),
               'tk': crypto_token}

    payload = '&'.join(['%s=%s' % (str(key), str(val)) for key,val in params.items()])

    with open(filename, 'wb') as file:
        request = urequests.get(_translate_url + 'translate_tts?' + payload,
                                headers=_GOOGLE_TTS_HEADERS)
        # TODO: read in chunks
        file.write(request.raw.read())
        request.raw.close()
    return True