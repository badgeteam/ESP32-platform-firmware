# File: otacheck.py
# Version: 1
# Description: OTA check module
# License: MIT
# Authors: Renze Nicolai <renze@rnplus.nl>

import easywifi, easydraw, badge, time, version

def checking_for_update_message():
    # Inform the user that we're checking for an update
    pass

def error_message():
    # Inform the user that an error occured
    pass

def download_info():
    import urequests as requests
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
        if not easywifi.status():
            if not easywifi.enable():
                return badge.nvs_get_u8('badge','OTA.ready',0)

        info = download_info()
        if info:
            if info["build"] > version.build:
                badge.nvs_set_u8('badge','OTA.ready',1)
                return True

        badge.nvs_set_u8('badge','OTA.ready',0)
    return badge.nvs_get_u8('badge','OTA.ready',0)
