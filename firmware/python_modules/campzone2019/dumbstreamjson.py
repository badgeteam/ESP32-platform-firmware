def _from_stream(stream, keys=None, blocksize=512):
    """
    Really dumb implementation of a tokenizer for JSON arrays of objects.
    Yields every item of the root array as a JSON object. Does not work on arrays of other types.
    :param stream: A file-like stream
    :param blocksize: An optional list of keys to extract. If set, items are returned with all entries but these stripped.
    :param keys: The number of bytes to read from the stream at once.
    :return:
    """
    import gc, ujson
    start = stream.read(1)
    start = start.decode() if type(start) is bytes else start
    if start != '[':
        raise ValueError('Content is not a JSON array of objects without unnecessary whitespace')

    buffer = stream.read(blocksize)
    object_counter = 0
    token = b''
    while buffer:
        for char in [b'%c' % char for char in buffer]:
            token += char
            if char == b'{':
                object_counter += 1
                if object_counter == 1:
                    token = b'{'
            elif char == b'}':
                object_counter -= 1
                if object_counter == 0:
                    item = ujson.loads(token.decode())
                    if keys is not None:
                        for key in item.keys():
                            if key not in keys:
                                del item[key]
                    print(item)
                    yield item
                    gc.collect()
        buffer = stream.read(blocksize)


def from_url(url, keys=None, blocksize=512):
    import urequests
    r = urequests.get(url)
    return _from_stream(r.raw, keys, blocksize)

from_file = _from_stream