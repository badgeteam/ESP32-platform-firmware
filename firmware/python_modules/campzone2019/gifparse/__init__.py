from . import gif

VERSION_TUPLE = (0, 0, 1)
VERSION = ".".join(map(str, VERSION_TUPLE))

def parse(raw):
    return gif.GIF(raw)
