import valuestore as store, uos as os, uinterface, time, system

def choose_apps():
    try:
        userApps = os.listdir('apps')
        userApps.reverse()
    except OSError:
        userApps = []
    apps = userApps
    apps.extend(['snake', 'clock', 'nickname'])
    prev_selected = store.load('slider', 'apps') or []
    uinterface.skippabletext('< > (de)select, A accept, B cancel')
    selected = uinterface.menu(apps, selected=prev_selected)
    if selected is not None:
        store.save('slider', 'apps', selected)

def set_slider_time():
    uinterface.skippabletext('Set seconds to show each app')
    seconds = uinterface.text_input(uinterface.NUMERIC_CHARSET)
    if seconds is not None and seconds != '':
        store.save('slider', 'time', int(seconds))

while True:
    function_index = uinterface.menu(['Choose slider apps', 'Set app slide time'])

    if function_index == 0:
        choose_apps()
    elif function_index == 1:
        set_slider_time()
    else:
        # B was pressed
        system.reboot()

    time.sleep(0.01)