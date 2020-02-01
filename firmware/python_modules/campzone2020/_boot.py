import uos, gc, sys, system, virtualtimers, machine

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

del folders, uos
gc.collect()
gc.mem_free()
