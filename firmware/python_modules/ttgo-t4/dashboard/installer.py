import orientation, dashboard.resources.woezel_repo as woezel_repo, term, easydraw, system, time, gc, ugfx, wifi, uos, json, sys, woezel, display, errno

repo = woezel_repo

orientation.default()

def showMessage(msg, error=False, icon_wifi=False, icon_ok=False):
	term.header(True, "Installer")
	print(msg)
	if error:
		easydraw.messageCentered("ERROR\n"+msg, True, "/media/alert.png")
	elif icon_wifi:
		easydraw.messageCentered("PLEASE WAIT\n"+msg, True, "/media/wifi.png")
	elif icon_ok:
		easydraw.messageCentered(msg, True, "/media/ok.png")
	else:
		easydraw.messageCentered("PLEASE WAIT\n"+msg, True, "/media/busy.png")

# Listbox element
myList = ugfx.List(0,0,ugfx.width(),ugfx.height())

# Generic actions
def btn_unhandled(pressed):
	display.flush(display.FLAG_LUT_FASTEST)

def btn_exit(pressed):
	if pressed:
		system.launcher()

def btn_update(pressed):
	if pressed:
		repo.update()
		system.start("dashboard.installer", True)

# Categories list

def show_categories(pressed = True, fromAppInstall = False):
	if not pressed:
		return
	ugfx.input_attach(ugfx.BTN_A, btn_unhandled)
	ugfx.input_attach(ugfx.BTN_B, btn_unhandled)
	ugfx.input_attach(ugfx.BTN_C, btn_unhandled)
	ugfx.input_attach(ugfx.BTN_BOOT, btn_unhandled)
	ugfx.clear(ugfx.WHITE)
	myList.clear()
	for category in repo.categories:
		myList.add_item("%s (%d) >" % (category["name"], category["eggs"]))
	myList.enabled(True)
	myList.visible(True)
	#Input handling
	ugfx.input_attach(ugfx.BTN_A, show_category)
	ugfx.input_attach(ugfx.BTN_B, btn_unhandled)
	ugfx.input_attach(ugfx.BTN_C, btn_unhandled)
	ugfx.input_attach(ugfx.BTN_BOOT, btn_exit)
	display.flush(display.FLAG_LUT_NORMAL)

# Category browsing

lastCategory = 0

def show_category(pressed=True, fromAppInstall = False):
	global lastCategory
	if not pressed:
		return
	ugfx.input_attach(ugfx.BTN_A, btn_unhandled)
	ugfx.input_attach(ugfx.BTN_B, btn_unhandled)
	ugfx.input_attach(ugfx.BTN_C, btn_unhandled)
	ugfx.input_attach(ugfx.BTN_BOOT, btn_unhandled)
	ugfx.clear(ugfx.WHITE)
	global category
	if not fromAppInstall:
		lastCategory = myList.selected_index()
	slug = repo.categories[lastCategory]["slug"]
	showMessage("Loading "+slug+"...")
	display.drawFill()
	myList.clear()
	try:
		try:
			category = repo.getCategory(slug)
		except BaseException as e:
			showMessage("Failed to open category "+slug+"!", True)
			display.drawFill()
			print("CAT OPEN ERR", e)
			time.sleep(1)
			show_categories()
			return
		gc.collect()
		for package in category:
			myList.add_item("%s rev. %s" % (package["name"], package["revision"]))
		myList.visible(True)
		myList.enabled(True)
		#Input handling
		ugfx.input_attach(ugfx.BTN_A, install_app)
		ugfx.input_attach(ugfx.BTN_B, btn_unhandled)
		ugfx.input_attach(ugfx.BTN_C, btn_unhandled)
		ugfx.input_attach(ugfx.BTN_BOOT, show_categories)
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
		slug = category[myList.selected_index()]["slug"]
		#Input handling
		ugfx.input_attach(ugfx.BTN_A, btn_unhandled)
		ugfx.input_attach(ugfx.BTN_B, btn_unhandled)
		ugfx.input_attach(ugfx.BTN_C, btn_unhandled)
		ugfx.input_attach(ugfx.BTN_BOOT, btn_unhandled)
		category = []
		myList.clear()
		showMessage("Installing "+slug+"...")
		with open("/cache/installList", "w") as f:
			f.write(slug)
		system.start("dashboard._installer_exec")

#Main application

showMessage("Loading categories...")
display.drawFill()
if not repo.load():
	if not repo.update():
		if repo.lastUpdate==0:
			showMessage("Failed to load repository. Returning to launcher...")
			display.drawFill()
			system.launcher()

show_categories()
