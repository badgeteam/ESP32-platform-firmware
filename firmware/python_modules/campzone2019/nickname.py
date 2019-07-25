import uinterface, system, machine, time
import rgb, buttons, defines

nickname = machine.nvs_getstr('badge', 'nickname')
should_set = False

def set_nickname():
    rgb.clear()
    rgb.scrolltext('Set nickname:')
    time.sleep(5)
    new_name = uinterface.text_input()
    if new_name is not None and new_name != '':
        machine.nvs_setstr('badge', 'nickname', new_name)
    system.start('nickname')


def render():
    rgb.clear()
    rgb.scrolltext(nickname)

def reset_nickname(_):
    global should_set
    should_set = True

if nickname is None or nickname == '':
    should_set = True

buttons.register(defines.BTN_A, reset_nickname)

if not should_set:
    render()

# Because we can't use uinterface from within interrupt handlers
while True:
    if should_set:
        set_nickname()
        render()
        should_set = False
    time.sleep(0.1)