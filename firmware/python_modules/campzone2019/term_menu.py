import term, badge, deepsleep as ds, system, version

class UartMenu():
	def __init__(self, gts, pm, safe = False, pol="Power off"):
		self.gts = gts
		self.menu = self.menu_main
		if (safe):
			self.menu = self.menu_safe
		self.buff = ""
		self.pm = pm
		self.power_off_label = pol
	
	def main(self):
		term.setPowerManagement(self.pm)
		while self.menu:
			self.menu()
		
	def drop_to_shell(self):
		self.menu = False
		term.clear()
		import shell
	
	def menu_main(self):
		items = ["Python shell", "Apps", "Installer", "Settings", "Tools", "About", "Check for updates", self.power_off_label]
		callbacks = [self.drop_to_shell, self.opt_launcher, self.opt_installer, self.menu_settings, self.menu_tools, self.opt_about, self.opt_ota_check, self.go_to_sleep]
		message = "Welcome!\nYour badge is running firmware version "+str(version.build)+": "+version.name+"\n"
		cb = term.menu("Main menu", items, 0, message)
		self.menu = callbacks[cb]
		return
	
	def go_to_sleep(self):
		self.gts()
		
	def opt_change_nickname(self):
		system.start("dashboard.terminal.nickname", True)
		
	def opt_installer(self):
		system.start("dashboard.terminal.installer", True)
	
	def opt_launcher(self):
		system.start("dashboard.terminal.launcher", True)
	
	def opt_configure_wifi(self):
		system.start("dashboard.terminal.wifi", True)
		
	def opt_configure_orientation(self):
		system.start("dashboard.terminal.orientation", True)
		
	def opt_ota(self):
		system.ota(True)
		
	def opt_ota_check(self):
		system.start("checkforupdates", True)
	
	def opt_about(self):
		system.start("about", True)
	
	def opt_configure_services(self):
		system.start("dashboard.terminal.services", True)
		
	def opt_configure_led(self):
		system.start("dashboard.terminal.ledservice", True)
	
	def opt_configure_picture(self):
		system.start("dashboard.terminal.picture", True)
		
	def opt_downloader(self):
		system.start("dashboard.terminal.downloader", True)
		
	def menu_settings(self):
		items = ["Change nickname", "Configure WiFi", "Set default orientation", "Homescreen services (beta)", "Homescreen LED animation", "Homescreen logo", "Update firmware", "< Return to main menu"]
		callbacks = [self.opt_change_nickname, self.opt_configure_wifi, self.opt_configure_orientation, self.opt_configure_services, self.opt_configure_led, self.opt_configure_picture, self.opt_ota, self.menu_main, self.menu_main]
		cb = term.menu("Settings", items)
		self.menu = callbacks[cb]
	
	def menu_tools(self):
		items = ["File downloader", "< Return to main menu"]
		callbacks = [self.opt_downloader, self.menu_main, self.menu_main]
		cb = term.menu("Tools", items)
		self.menu = callbacks[cb]
	
	def menu_safe(self):
		items = ["Main menu"]
		callbacks = [self.menu_main]
		cb = term.menu("You have started the badge in safe mode!", items)
		self.menu = callbacks[cb]
