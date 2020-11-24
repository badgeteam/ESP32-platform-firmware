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
    """
    stat.ST_MODE
    Inode protection mode.

    stat.ST_INO
    Inode number.

    stat.ST_DEV
    Device inode resides on.

    stat.ST_NLINK
    Number of links to the inode.

    stat.ST_UID
    User id of the owner.

    stat.ST_GID
    Group id of the owner.

    stat.ST_SIZE
    Size in bytes of a plain file; amount of data waiting on some special files.

    stat.ST_ATIME
    Time of last access.

    stat.ST_MTIME
    Time of last modification.

    stat.ST_CTIME
    The “ctime” as reported by the operating system. On some systems (like Unix) is the time of the last metadata
    change, and, on others (like Windows), is the creation time (see platform documentation for details).

    :param filepath:
    :return: Tuple with above values
    """
    filepath = _absolute_path(filepath)
    return _os.stat(filepath)

# Add some nice standard Python path traversal methods
class path:
    def isdir(path):
        try:
            st = stat(path)
            return not not (st[0] & 0x4000)
        except OSError as e:
            error_code = e.args[0]
            if error_code == 2:
                return False
            else:
                raise e

    def isfile(path):
        try:
            st = stat(path)
            return not (st[0] & 0x4000)
        except OSError as e:
            error_code = e.args[0]
            if error_code == 2:
                return False
            else:
                raise e

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