import term, system, machine

system.serialWarning()

term.header(True, "Configure nickname")
nickname = machine.nvs_getstr("owner", "name")
if not nickname:
	nickname = ""
nickname = term.prompt("Nickname", 1, 3, nickname)
machine.nvs_setstr("owner", "name", nickname)
system.home()
