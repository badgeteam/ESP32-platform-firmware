import machine, system
machine.nvs_setstr("system", 'default_app', "")
system.eraseStorage()
