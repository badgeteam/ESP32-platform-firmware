import orientation, dashboard.resources.woezel_repo as woezel_repo, term, easydraw, system, time, gc, ugfx, wifi, uos, json, sys, woezel, display

repo = woezel_repo

orientation.default()

def showMessage(msg, error=False, icon_wifi=False, icon_ok=False):
	term.header(True, "Installer")
	print(msg)
	if error:
		easydraw.messageCentered("ERROR\n\n"+msg, True, "/media/alert.png")
	elif icon_wifi:
		easydraw.messageCentered("PLEASE WAIT\n\n"+msg, True, "/media/wifi.png")
	elif icon_ok:
		easydraw.messageCentered(msg, True, "/media/ok.png")
	else:
		easydraw.messageCentered("PLEASE WAIT\n\n"+msg, True, "/media/busy.png")

# Generic actions
def btn_unhandled(pressed):
	display.flush(display.FLAG_LUT_FASTEST)

def btn_exit(pressed):
	if pressed:
		system.launcher()

def btn_update(pressed):
	if pressed:
		repo.update()
		system.start("installer", True)

# Categories list

categories_list = ugfx.List(0,0,ugfx.width(),ugfx.height()-48)

def show_categories(pressed=True):
	if not pressed:
		return
	ugfx.clear(ugfx.WHITE)
	#Hide category list
	category_list.visible(False)
	category_list.enabled(False)
	#Show categories list
	categories_list.visible(True)
	categories_list.enabled(True)
	#Input handling
	ugfx.input_attach(ugfx.BTN_START, btn_exit)
	ugfx.input_attach(ugfx.BTN_SELECT, btn_update)
	ugfx.input_attach(ugfx.BTN_A, show_category)
	ugfx.input_attach(ugfx.BTN_B, btn_unhandled)
	ugfx.input_attach(ugfx.JOY_UP, btn_unhandled)
	ugfx.input_attach(ugfx.JOY_DOWN, btn_unhandled)
	ugfx.input_attach(ugfx.JOY_LEFT, btn_unhandled)
	ugfx.input_attach(ugfx.JOY_RIGHT, btn_unhandled)
	#Hint
	easydraw.disp_string_right_bottom(0, "START: Exit app")
	easydraw.disp_string_right_bottom(1, "A: Open category")
	easydraw.disp_string_right_bottom(2, "SELECT: Update repo")
	#Flush screen
	display.flush(display.FLAG_LUT_NORMAL)

# Category browsing

category_list   = ugfx.List(0,0,ugfx.width(),ugfx.height()-48)

def show_category(pressed=True):
	if not pressed:
		return
	ugfx.clear(ugfx.WHITE)
	global category
	categories_list.visible(False)
	categories_list.enabled(False)
	slug = repo.categories[categories_list.selected_index()]["slug"]
	showMessage("Loading "+slug+"...")
	display.drawFill()
	#Clean up list
	while category_list.count() > 0:
		category_list.remove_item(0)
	try:
		try:
			category = repo.getCategory(slug)
		except BaseException as e:
			print("CAT OPEN ERR", e)
			showMessage("Failed to open category "+slug+"!", True)
			display.drawFill()
			time.sleep(1)
			show_categories()
		gc.collect()
		for package in category:
			category_list.add_item("%s rev. %s" % (package["name"], package["revision"]))
		category_list.selected_index(0)
		category_list.visible(True)
		category_list.enabled(True)
		#Input handling
		ugfx.input_attach(ugfx.BTN_START, btn_exit)
		ugfx.input_attach(ugfx.BTN_SELECT, btn_unhandled)
		ugfx.input_attach(ugfx.BTN_A, install_app)
		ugfx.input_attach(ugfx.BTN_B, show_categories)
		ugfx.input_attach(ugfx.JOY_UP, btn_unhandled)
		ugfx.input_attach(ugfx.JOY_DOWN, btn_unhandled)
		ugfx.input_attach(ugfx.JOY_LEFT, btn_unhandled)
		ugfx.input_attach(ugfx.JOY_RIGHT, btn_unhandled)
		#Hint
		easydraw.disp_string_right_bottom(0, "START: Exit")
		easydraw.disp_string_right_bottom(1, "A: Install app")
		easydraw.disp_string_right_bottom(2, "B: Back")
		#Flush screen
		display.flush(display.FLAG_LUT_NORMAL)
	except BaseException as e:
		sys.print_exception(e)
		print("ERROR", e)
		showMessage("Internal error", True)
		display.drawFill()
		time.sleep(1)
		show_categories()

# Install application

def install_app(pressed=True):
	global category	
	if pressed:
		slug = category[category_list.selected_index()]["slug"]
		category = []
		gc.collect()
		category_list.visible(False)
		category_list.enabled(False)
		category_list.clear()
		#Input handling
		ugfx.input_attach(ugfx.BTN_START, btn_unhandled)
		ugfx.input_attach(ugfx.BTN_SELECT, btn_unhandled)
		ugfx.input_attach(ugfx.BTN_A, btn_unhandled)
		ugfx.input_attach(ugfx.BTN_B, btn_unhandled)
		ugfx.input_attach(ugfx.JOY_UP, btn_unhandled)
		ugfx.input_attach(ugfx.JOY_DOWN, btn_unhandled)
		ugfx.input_attach(ugfx.JOY_LEFT, btn_unhandled)
		ugfx.input_attach(ugfx.JOY_RIGHT, btn_unhandled)
		if not wifi.status():
			wifi.connect()
			wifi.wait()
			if not wifi.status():
				showMessage("Unable to connect to WiFi.")
				display.drawFill()
				time.sleep(2)
				show_category()
		showMessage("Installing "+slug+"...")
		display.drawFill()
		try:
			woezel.install(slug)
			showMessage("OK\n\n"+slug+" has been installed!", False, False, True)
			display.drawFill()
			time.sleep(2)
			show_category()
		except woezel.LatestInstalledError:
			showMessage("NOTICE\n\nLatest version is already installed.", False, False, True)
			display.drawFill()
			time.sleep(2)
			show_category()
		except BaseException as e:
			print("WOEZEL ERROR", e)
			showMessage("Failed to install "+slug+"!", True)
			display.drawFill()
			time.sleep(2)
			show_category()

#Main application

showMessage("Loading categories...")
display.drawFill()
if not repo.load():
	if not repo.update():
		if repo.lastUpdate==0:
			showMessage("Failed to load repository. Returning to launcher...")
			display.drawFill()
			system.launcher()

for category in repo.categories:
	categories_list.add_item("%s (%d) >" % (category["name"], category["eggs"]))

show_categories()
