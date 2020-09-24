import system, badge, keyboard

def callback(value):
    if value:
        badge.nvs_set_str("owner", "name", value)
    system.home()

nickname = badge.nvs_get_str("owner", "name", "")
keyboard.show("Nickname", nickname, callback)
