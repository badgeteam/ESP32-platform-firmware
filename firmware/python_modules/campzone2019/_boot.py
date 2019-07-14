import uos, gc, sys

folders = ['lib', 'apps', 'cache', 'cache/woezel', 'config']
for folder in folders:
    print("Creating folder {}".format(folder))
    try:
        uos.mkdir(folder)
        print("Created!")
    except Exception as error:
        print("Error: %s" % error)

# This doesn't work in micropython/main.c because micropython can't handle
# slash characters before single characters that are also HTML elements,
# like <a> or <s> (e.g. /apps or /sdcard won't work.)
sys.path.append('apps')

gc.collect()
