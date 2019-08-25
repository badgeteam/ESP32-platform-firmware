import gc
import uos
import ohsbadge
from flashbdev import bdev

try:
    if bdev:
        uos.mount(bdev, '/')
except OSError:
    import inisetup
    vfs = inisetup.setup()

gc.collect()
ohsbadge.start()
