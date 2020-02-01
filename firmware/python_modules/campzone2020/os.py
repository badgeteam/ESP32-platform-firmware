import uos as _os

# Expose all uos methods through this library
for name in dir(_os):
    globals()[name] = getattr(_os, name)

def _absolute_path(filepath='.'):
    # Replace relative with absolute path to prevent
    # issues stat()ing any mounted SD card overlay
    if filepath[0] == ".":
        filepath = _os.getcwd() + filepath[1:]
    if filepath.startswith('//'):
        filepath = filepath[1:]
    return filepath


def listdir(dir='.'):
    dir = _absolute_path(dir)
    return _os.listdir(dir)


def stat(filepath='.'):
    filepath = _absolute_path(filepath)
    return _os.stat(filepath)

# Add some nice standard Python path traversal methods
class path:
    def isdir(path):
        st = stat(path)
        return not not (st[0] & 0x4000)

    def join(path, *paths):
        result = path
        for _path in paths:
            if result[-1] != '/':
                result += '/'
            result += _path
        return result

## Non-standard functions that are useful
def listfiles(filepath='.', return_full=False):
    filepath = _absolute_path(filepath)
    names = listdir(filepath)
    return [path.join(filepath, name) if return_full else name for name in names
            if not path.isdir(path.join(filepath, name))]