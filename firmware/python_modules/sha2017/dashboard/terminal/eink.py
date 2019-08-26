import machine, system, term

system.serialWarning()

def load():
	return machine.nvs_get_u8('system', 'eink.dev.type')

while True:
	current = load()
	if current == None:
		option = 0
	elif current == 0:
		option = 1 #DISABLE
	elif current == 1:
		option = 2 #ALT
	elif current == 2:
		option = 3 #SHA
	else:
		# Invalid value, set SHA2017 type
		option = 3
		machine.nvs_set_u8('system', 'eink.dev.type', 2)
	items = []
	items.append("Use default")
	items.append("Disable display driver")
	items.append("GDEH029A1 (replacement)")
	items.append("DEPG0290B1 (SHA2017)")
	items.append("< Back")
	option = term.menu("Splash screen settings", items, option, "", 64)
	if option == 0:
		try:
			machine.nvs_erase("system", "eink.dev.type")
		except:
			pass
	if option == 1:
		machine.nvs_set_u8('system', 'eink.dev.type', 0)
	if option == 2:
		machine.nvs_set_u8('system', 'eink.dev.type', 1)
	if option == 3:
		machine.nvs_set_u8('system', 'eink.dev.type', 2)
	system.home()
