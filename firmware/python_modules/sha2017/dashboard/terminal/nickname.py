import term, system, badge, easydraw

system.serialWarning()

term.header(True, "Configure nickname")
nickname = badge.nvs_get_str("owner", "name", "")
nickname = term.prompt("Nickname", 1, 3, nickname)
badge.nvs_set_str("owner", "name", nickname)
system.home()
