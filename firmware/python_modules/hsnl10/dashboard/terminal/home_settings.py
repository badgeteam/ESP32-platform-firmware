import machine, system, term

system.serialWarning()

def load():
	cfg_term_menu = machine.nvs_get_u8('splash', 'term_menu')
	if cfg_term_menu == None:
		cfg_term_menu = True

	cfg_wifi = machine.nvs_get_u8('splash', 'wifi') # Allow the use of WiFi on the splash screen
	if cfg_wifi == None:
		cfg_wifi = False # If not set the use of WiFi is not allowed

	cfg_services = machine.nvs_get_u8('splash', 'services') # Enable splash screen services (fun but dangerous)
	if cfg_services == None:
		cfg_services = False # If not set services are disabled

	cfg_logo = machine.nvs_getstr('splash', 'logo') # Filename of a PNG image to show on the splash screen

	cfg_nickname = machine.nvs_get_u8('splash', 'nickname') # Show the nickname of the user on the splash screen
	if cfg_nickname == None:
		cfg_nickname = True # If not set we want to show the nickname

	cfg_greyscale = machine.nvs_get_u8('splash', 'greyscale') # Use greyscale mode
	if cfg_greyscale == None:
		cfg_greyscale = False # Disabled by default

	cfg_led_animation = machine.nvs_getstr('splash', 'ledApp') # Application which shows a LED animation while the splash screen is visible
	
	return cfg_term_menu, cfg_wifi, cfg_services, cfg_logo, cfg_nickname, cfg_greyscale, cfg_led_animation

option = 0
while True:
	cfg_term_menu, cfg_wifi, cfg_services, cfg_logo, cfg_nickname, cfg_greyscale, cfg_led_animation = load()
	items = []
	if cfg_term_menu:
		items.append("MENU         : Enabled")
	else:
		items.append("MENU         : Disabled")
	if cfg_services:
		items.append("SERVICES     : Enabled")
	else:
		items.append("SERVICES     : Disabled")
	if cfg_wifi:
		items.append("WIFI         : OTA check enabled")
	else:
		items.append("WIFI         : OTA check disabled")
	if cfg_nickname:
		items.append("NICKNAME:    : Shown")
	else:
		items.append("NICKNAME     : Hidden")
	if cfg_greyscale:
		items.append("RENDERING    : Greyscale (slow)")
	else:
		items.append("RENDERING    : Black & white (quick)")
	if cfg_logo:
		items.append("LOGO         : "+cfg_logo)
	else:
		items.append("LOGO         : None")
	if cfg_led_animation:
		items.append("LED ANIMATION: "+cfg_led_animation)
	else:
		items.append("LED ANIMATION: None")
	items.append("< Back")
	option = term.menu("Splash screen settings", items, option, "", 64)
	if option == 0:
		machine.nvs_set_u8('splash', 'term_menu', not cfg_term_menu)
	elif option == 1:
		machine.nvs_set_u8('splash', 'services', not cfg_services)
	elif option == 2:
		machine.nvs_set_u8('splash', 'wifi', not cfg_wifi)
	elif option == 3:
		machine.nvs_set_u8('splash', 'nickname', not cfg_nickname)
	elif option == 4:
		machine.nvs_set_u8('splash', 'greyscale', not cfg_greyscale)
	elif option == 5:
		term.header(True, "Logo path")
		if not cfg_logo:
			cfg_logo = ""
		cfg_logo = term.prompt("Logo path", 1, 3, cfg_logo)
		if not cfg_logo:
			try:
				machine.nvs_erase("splash", "logo")
			except:
				pass
		else:
			machine.nvs_setstr('splash', 'logo', cfg_logo)
	elif option == 6:
		term.header(True, "LED animation app")
		if not cfg_led_animation:
			cfg_led_animation = ""
		cfg_led_animation = term.prompt("App name", 1, 3, cfg_led_animation)
		if not cfg_led_animation:
			try:
				machine.nvs_erase("splash", "ledApp")
			except:
				pass
		else:
			machine.nvs_setstr('splash', 'ledApp', cfg_led_animation)
	else:
		system.home()
