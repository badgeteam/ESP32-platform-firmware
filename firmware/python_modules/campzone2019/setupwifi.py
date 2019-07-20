import rgb, network, machine, time, buttons, defines, system
from default_icons import animation_connecting_wifi

chosen_ssid = ''
chosen_pass = 'A'

cur_selected = 0
accepted = False

data, size, frames = animation_connecting_wifi
rgb.clear()
rgb.framerate(3)
rgb.gif(data, (12, 0), size, frames)

sta_if = network.WLAN(network.STA_IF)
sta_if.active(True)
sta_if.disconnect()
ssid_result = sta_if.scan()
ssid_list = [ssid[0].decode('utf-8', 'ignore') for ssid in ssid_result]


def render_ssid():
    rgb.clear()
    rgb.scrolltext(ssid_list[cur_selected])

def up_1(pressed):
    global cur_selected
    if pressed:
        cur_selected = max(cur_selected - 1, 0)
        render_ssid()

def down_1(pressed):
    global cur_selected
    if pressed:
        cur_selected = min(cur_selected + 1, len(ssid_list))
        render_ssid()

def ok(pressed):
    global accepted
    if pressed:
        accepted = True

def render_pass():
    rgb.clear()
    rgb.text(chosen_pass)


def up_2(pressed):
    global chosen_pass
    if pressed:
        new_char = chr(ord(chosen_pass[cur_selected]) + 1)
        chosen_pass = chosen_pass[0:cur_selected] + new_char + chosen_pass[cur_selected+1:]
        render_pass()

def down_2(pressed):
    global chosen_pass
    if pressed:
        new_char = chr(ord(chosen_pass[cur_selected]) - 1)
        chosen_pass = chosen_pass[0:cur_selected] + new_char + chosen_pass[cur_selected+1:]
        render_pass()

def left(pressed):
    global cur_selected
    if pressed:
        cur_selected = max(cur_selected - 1, 0)
        render_pass()

def right(pressed):
    global cur_selected
    global chosen_pass
    if pressed:
        cur_selected += 1
        print(cur_selected, len(chosen_pass))
        if cur_selected >= len(chosen_pass):
            chosen_pass += 'A'
            render_pass()

def back(pressed):
    global chosen_pass
    if pressed:
        if len(chosen_pass) > 1:
            chosen_pass = chosen_pass[:-1]
            render_pass()
        else:
            system.reboot()

rgb.clear()
rgb.framerate(20)
rgb.setfont(rgb.FONT_6x3)
rgb.scrolltext('Select network')
time.sleep(5)
render_ssid()

buttons.register(defines.BTN_UP, up_1)
buttons.register(defines.BTN_DOWN, down_1)
buttons.register(defines.BTN_A, ok)
buttons.register(defines.BTN_B, back)

while not accepted:
    time.sleep(0.1)

chosen_ssid = ssid_list[cur_selected]

buttons.unassign(defines.BTN_UP)
buttons.unassign(defines.BTN_DOWN)
accepted = False
cur_selected = 0
rgb.clear()
rgb.scrolltext('Enter password:')
time.sleep(5)
render_pass()

buttons.assign(defines.BTN_UP, up_2)
buttons.assign(defines.BTN_DOWN, down_2)
buttons.register(defines.BTN_LEFT, left)
buttons.register(defines.BTN_RIGHT, right)

while not accepted:
    time.sleep(0.1)

machine.nvs_setstr("system", "wifi.ssid", chosen_ssid)
machine.nvs_setstr("system", "wifi.password", chosen_pass)
system.reboot()