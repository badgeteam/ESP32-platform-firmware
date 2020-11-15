import system, rgb, time, gc, urequests as requests, version
import uinterface, consts
from uinterface import confirmation_dialog, connect_wifi

_FONT = rgb.FONT_7x5
_MAIN_FRAMERATE = 20

def main():
    try:
        _initialize_display()
        latest_update = _retrieve_latest_update()
        if _prompt_user_for_update(latest_update["name"], latest_update["build"]):
            uinterface.skippabletext("Starting update")
            system.ota()
        else:
            uinterface.skippabletext("Cancelled OTA update")
    except Exception as e:
        rgb.clear()
        uinterface.skippabletext(str(e))

def _retrieve_latest_update():
    if not connect_wifi():
            raise ValueError("No WiFi connection")
    try:
        _show_progress("Checking for updates")
        gc.collect()
        ota_version_url = 'https://' + consts.OTA_WEB_SERVER + ':' + consts.OTA_WEB_PORT.replace('"', '') + '/' + consts.OTA_WEB_VERSION_PATH
        request = requests.get(ota_version_url, timeout=10)
        result = request.json()
        request.close()
        update_information = {
            "name": result["name"],
            "build": result["build"]
        }
        return update_information
    except Exception as e:
        raise Exception("Update check failed")

def _prompt_user_for_update(name, build):

    current = str(consts.INFO_FIRMWARE_BUILD)
    to_install = str(build)

    int_current = int(current)
    int_to_install = int(to_install)

    short_current = current[6:] if len(current) > 6 else current
    short_to_install = to_install[6:] if len(to_install) > 6 else to_install

    action = 'Downgrade' if int_current > int_to_install else ('Upgrade' if int_current < int_to_install else 'Reinstall')
    text = '%s version %s to %s?' % (action, short_current, short_to_install) if action != 'Reinstall' else \
        'No newer update available, reinstall existing firmware?'

    return _confirm_ota(text)

def _confirm_ota(text):
    return confirmation_dialog(text)

def _show_progress(text):
    rgb.clear()
    rgb.scrolltext(text)

def _initialize_display():
    rgb.clear()
    rgb.setfont(_FONT)
    rgb.framerate(_MAIN_FRAMERATE)

main()
system.home()
