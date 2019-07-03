import ugfx, time, badge, machine, deepsleep, gc
import appglue, virtualtimers
import easydraw, easywifi, easyrtc

import tasks.powermanagement as pm
import tasks.otacheck as otac
import tasks.resourcescheck as resc
import tasks.sponsorscheck as spoc

import post_ota

def start():
    if badge.safe_mode():
        splash = 'splash'
    else:
        splash = badge.nvs_get_str('boot','splash','splash')
        if splash.startswith('bpp '):
            splash = splash[4:len(splash)]
            badge.mount_bpp()
        elif splash.startswith('sdcard '):
            splash = splash[7:len(splash)]
            badge.mount_sdcard()
        __import__(splash)

setupState = badge.nvs_get_u8('badge', 'setup.state', 0)
if setupState == 0: #First boot
    print("First boot (start setup)...")
    __import__("setup")
elif setupState == 1: # Second boot: Show sponsors
    print("Second boot (show sponsors)...")
    badge.nvs_set_u8('badge', 'setup.state', 2)
    spoc.show(True)
elif setupState == 2: # Third boot: force OTA check
    print("Third boot (force ota check)...")
    badge.nvs_set_u8('badge', 'setup.state', 3)
    if not easywifi.failure():
        otac.available(True)
    start()
else: # Normal boot
    print("Normal boot... ("+str(machine.reset_cause())+")")
    if (machine.reset_cause() != machine.DEEPSLEEP_RESET):
        print("... from reset: checking for ota update")
        if not easywifi.failure():
            otac.available(True)
        start()

