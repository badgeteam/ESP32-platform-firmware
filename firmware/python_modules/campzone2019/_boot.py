import uos, gc, sys, system, rgb, time

folders = ['lib', 'apps', 'cache', 'cache/woezel', 'config']
for folder in folders:
    try:
        uos.mkdir(folder)
    except Exception as error:
        pass

# This doesn't work in micropython/main.c because micropython can't handle
# slash characters before single characters that are also HTML elements,
# like <a> or <s> (e.g. /apps or /sdcard won't work.)
sys.path.append('apps')

# Hijack the system start function to fix some CZ19 screen issues
orig_start = system.start
def hijacked_start(app, status=True):
    rgb.clear()
    time.sleep(1 / 20  * 1.1)  # 110% of the time of one render frame
    orig_start(app, status)
system.start = hijacked_start

del folders, uos, time
gc.collect()
