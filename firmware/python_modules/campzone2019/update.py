import system, rgb, time, gc, urequests as requests, version
from uinterface import menu, confirmation_dialog, connect_wifi

_FONT = rgb.FONT_7x5
_MAIN_FRAMERATE = 20

def main():
    try:
        _initialize_display()
        latest_update = _retrieve_latest_update()
        if _prompt_user_for_update(latest_update["name"], latest_update["build"]):
            _show_progress("Starting update")
            time.sleep(5)
            system.ota()
        else:
            _show_progress("Cancelled OTA update")
            time.sleep(6)

    except Exception as e:
        _show_progress(str(e))
        time.sleep(6)

def _retrieve_latest_update():
    if not connect_wifi():
            raise ValueError("No WiFi connection")
    try:
        _show_progress("Checking for updates")
        gc.collect()
        request = requests.get(version.otacheckurl, timeout=30)
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
    template = "You have {} firmware. Confirm {}?"
    if build > version.build:
            prompt = template.format("a newer", "downgrade")
    elif build == version.build:
        prompt = template.format("the latest", "reinstall")
    else:
        prompt = "An upgrade to version {}, \"{}\" is available! Confirm upgrade?".format(build, name)

    return _confirm_ota(prompt)

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
system.reboot()