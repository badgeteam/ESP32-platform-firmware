import woezel, wifi, machine, system, gc

def install(app_name):
    machine.nvs_setstr('uinstaller', 'to_install', app_name)
    system.start('uinstaller')


if system.__current_app__ == 'uinstaller':
    to_install = machine.nvs_getstr('uinstaller', 'to_install')
    machine.nvs_setstr('uinstaller', 'to_install', '')
    print('Installing %s' % to_install)
    if not to_install:
        system.reboot()

    wifi.connect()
    if not wifi.wait():
        system.reboot()

    print('Installing %s' % to_install)
    gc.collect()
    if woezel.install(to_install):
        # Reset launcher's selected index to newly installed app
        machine.nvs_setint('launcher', 'index', 0)
        print('Install succeeded')
    else:
        print('Failed to install "%s"' % to_install)
        
    system.reboot()