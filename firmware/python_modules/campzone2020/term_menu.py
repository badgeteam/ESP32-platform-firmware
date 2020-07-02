import term, system, consts

class UartMenu():
	def __init__(self, gts = None, pm = None, safe = False, pol="Power off"):
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
		items = ["Python shell", "Apps", "Installer", "Settings", "About", "Check for updates", "Tools"]
		callbacks = [self.drop_to_shell, self.opt_launcher, self.opt_installer, self.menu_settings, self.opt_about, self.opt_ota_check]
		message = "Welcome!\nYour badge is running firmware version "+str(consts.INFO_FIRMWARE_BUILD)+": "+consts.INFO_FIRMWARE_NAME+"\n"
		cb = term.menu("Main menu", items, 0, message)
		self.menu = callbacks[cb]
		return
		
	def opt_change_nickname(self):
		system.start("dashboard.terminal.nickname", True)
		
	def opt_installer(self):
		system.start("dashboard.terminal.installer", True)
	
	def opt_launcher(self):
		system.start("dashboard.terminal.launcher", True)
	
	def opt_configure_wifi(self):
		system.start("dashboard.terminal.wifi", True)
		
	def opt_ota(self):
		system.ota(True)
		
	def opt_ota_check(self):
		system.start("checkforupdates", True)
	
	def opt_about(self):
		system.start("about", True)
		
	def menu_settings(self):
		items = ["Configure WiFi", "< Return to main menu"]
		callbacks = [self.opt_configure_wifi, self.menu_main]
		cb = term.menu("Settings", items)
		self.menu = callbacks[cb]
	
	def menu_safe(self):
		items = ["Main menu"]
		callbacks = [self.menu_main]
		cb = term.menu("You have started the badge in safe mode!", items)
		self.menu = callbacks[cb]
