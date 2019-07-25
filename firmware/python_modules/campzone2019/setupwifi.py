import rgb, network, machine, time, buttons, defines, system, uinterface
from default_icons import animation_connecting_wifi

def scan_access_point_list():
        data, size, frames = animation_connecting_wifi
        rgb.clear()
        rgb.framerate(3)
        rgb.gif(data, (12, 0), size, frames)
        sta_if = network.WLAN(network.STA_IF)
        sta_if.active(True)
        sta_if.disconnect()
        ap_result = sta_if.scan()
        return [(ap[0].decode("utf-8", "ignore"), ap[5]) for ap in ap_result]

def prompt_message(message):
        rgb.clear()
        rgb.framerate(20)
        rgb.setfont(rgb.FONT_7x5)
        rgb.scrolltext(message)
        time.sleep(5)

def ap_requires_password(ap_type):
        return "OPEN" != ap_type

ap_list = scan_access_point_list()
ssids = [ap[0] for ap in ap_list]
prompt_message("Select network")

choice = uinterface.menu(ssids)
if not (choice is None):
        chosen_ssid, chosen_ap_type = ap_list[choice]

        pw_required = ap_requires_password(chosen_ap_type)
        if pw_required:
                prompt_message("Enter password")

        chosen_pass = uinterface.text_input() if pw_required else ''
        if not pw_required or chosen_pass:
                machine.nvs_setstr("system", "wifi.ssid", chosen_ssid)
                machine.nvs_setstr("system", "wifi.password", chosen_pass)

system.reboot()