import term, appglue, sys, badge, easydraw

easydraw.msg("This app can only be controlled using the USB-serial connection.", "Notice", True)

term.header(True, "Configure nickname")
nickname = badge.nvs_get_str("owner", "name", "")
nickname = term.prompt("Nickname", 1, 3, nickname)
badge.nvs_set_str("owner", "name", nickname)
appglue.home()
