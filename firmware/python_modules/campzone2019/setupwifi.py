import rgb, network, machine, time, buttons, defines, system, uinterface
from default_icons import animation_connecting_wifi

def scan_ssid_list():
        data, size, frames = animation_connecting_wifi
        rgb.clear()
        rgb.framerate(3)
        rgb.gif(data, (12, 0), size, frames)
        sta_if = network.WLAN(network.STA_IF)
        sta_if.active(True)
        sta_if.disconnect()
        ssid_result = sta_if.scan()
        return [ssid[0].decode('utf-8', 'ignore') for ssid in ssid_result]

def prompt_message(message):
        rgb.clear()
        rgb.framerate(20)
        rgb.setfont(rgb.FONT_7x5)
        rgb.scrolltext(message)
        time.sleep(5)

ssid_list = scan_ssid_list()
prompt_message('Select network')

choice = uinterface.menu(ssid_list)
if not (choice is None):
        chosen_ssid = ssid_list[choice]
        prompt_message('Enter password')
        chosen_pass = uinterface.text_input()
        if chosen_pass:
                machine.nvs_setstr("system", "wifi.ssid", chosen_ssid)
                machine.nvs_setstr("system", "wifi.password", chosen_pass)

system.reboot()