import system, machine, keyboard

def callback(value):
    if value:
        machine.nvs_setstr("owner", "name", value)
    system.home()

nickname = machine.nvs_getstr("owner", "name")
if nickname == None:
	nickname = ""
keyboard.show("Nickname", nickname, callback)
