import sndmixer, urequests

_no_channels = 4
sndmixer.begin(_no_channels)
handles = {}

def _clean_channel(channel_id):
    global handles
    if channel_id in handles:
        file = handles[channel_id]
        if file is not None:
            file.close()
        del handles[channel_id]

def _add_channel(filename_or_url):
    global handles
    stream = True
    is_url = filename_or_url.startswith('http')
    file = open(filename_or_url, 'rb') if not is_url else urequests.get(filename_or_url).raw
    lower = filename_or_url.lower()
    if(lower.endswith('.mp3')):
        channel_id = sndmixer.mp3_stream(file)
    elif(lower.endswith('.wav')):
        channel_id = sndmixer.wav_stream(file)
    elif(lower.endswith('.ogg') or
         lower.endswith('.opus')):
        channel_id = sndmixer.opus_stream(file)  # No support for looping yet
    elif(lower.endswith('.mod') or
         lower.endswith('.s3m') or
         lower.endswith('.xm')):
        channel_id = sndmixer.mod(file.read())  # No support for streaming mod files or looping yet
        stream = False
    else:
        print('Audio: unknown filetype')
        channel_id = -1

    if channel_id < 0:
        return -1

    if is_url:
        # Needed when streaming from HTTP sockets
        sndmixer.play(channel_id)

    handles[channel_id] = file
    if stream:
        sndmixer.on_finished(channel_id, lambda _ : _clean_channel(channel_id))
    return channel_id

def play(filename_or_url, volume=255, loop=False, sync_beat=None, start_at_next=None):
    channel_id = _add_channel(filename_or_url)
    if channel_id is None or channel_id < 0:
        print('Failed to start audio channel')
        return channel_id
    sndmixer.volume(channel_id, volume)
    if loop:
        sndmixer.loop(channel_id, True)
    if sync_beat is not None and start_at_next is not None:
        sndmixer.start_beat_sync(sync_beat)
        sndmixer.start_at_next(channel_id, start_at_next)
    else:
        sndmixer.play(channel_id)
    return channel_id

def stop_channel(channel_id):
    _clean_channel(channel_id)
    sndmixer.stop(channel_id)

def stop_looping(channel_id):
    sndmixer.loop(channel_id, False)