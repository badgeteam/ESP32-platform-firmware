import machine, virtualtimers, system, gc, valuestore

apps = valuestore.load('slider', 'apps') or ['clock', 'nickname']
current_index = valuestore.load('slider', 'current') or 0
time = valuestore.load('slider', 'time') or 20
next_index = None if current_index is None else ((current_index + 1) % len(apps))

def _next():
    import valuestore
    ## Start next app by restarting slider
    print('rebooting into next app: ', next_index)
    valuestore.save('slider', 'current', next_index)
    system.start('slider')
    pass

## Set timer for starting next app
virtualtimers.new(time * 1000, _next, hfpm=True)

if len(apps) == 0:
    print('No apps set, slider will start normal launcher instead')
    system.reboot()
if current_index is None or current_index < 0 or current_index >= len(apps):
    print('Current app index %d is out of the bounds of the set slider apps, setting index to 0' % current_index)
    try:
        current_index = 0
        valuestore.save('slider', 'current', current_index)
    except BaseException as error:
        print('Additional error whilst saving index: %s' % error)
        system.start('launcher')

app = apps[current_index]
del apps, current_index, valuestore
gc.collect()

## Start current app
try:
    print("Starting app '%s'..." % app)
    system.__current_app__ = app
    if app:
        __import__(app)
except BaseException as e:
    sys.print_exception(e)
    if not machine.nvs_get_u8("system", "ignore_crash"):
        print("Fatal exception in the running app!")
        system.crashedWarning()
        time.sleep(3)
        system.launcher()