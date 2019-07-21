# File: otacheck.py
# Version: 1
# Description: OTA check module
# License: MIT
# Authors: Renze Nicolai <renze@rnplus.nl>

import machine, time, version

def checking_for_update_message():
    # Inform the user that we're checking for an update
    pass

def error_message():
    # Inform the user that an error occured
    pass

def download_info():
    import requests
    checking_for_update_message()
    result = None
    try:
        data = requests.get(version.otacheckurl)
        result = data.json()
        result['build'] = int(result['build'])
        result['name'] = str(result['name'])
        data.close()
    except:
        error_message()
        time.sleep(5)
    return result

def available(update=False):
    if update:
        if not wifi.status():
            wifi.connect()
            if not wifi.wait():
                return machine.nvs_getint('badge','OTA.ready') or 0

        info = download_info()
        if info:
            if info["build"] > version.build:
                machine.nvs_setint('badge','OTA.ready', 1)
                return True

        machine.nvs_setint('badge','OTA.ready', 0)
    return machine.nvs_getint('badge','OTA.ready') or 0
