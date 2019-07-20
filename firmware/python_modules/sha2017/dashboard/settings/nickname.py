import system, badge, dialogs

def callback(value):
    if value:
        badge.nvs_set_str("owner", "name", value)
    system.home()

nickname = badge.nvs_get_str("owner", "name", "")
dialogs.prompt_text("Nickname", nickname, cb=callback)
