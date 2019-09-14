# Application repository browser
# Disobey 2019 version
# Last updated on 13-09-2019

import orientation, easydraw, display, ugfx           # Display & graphics
import system, time, gc, sys                          # Generic
import dashboard.resources.woezel_repo as woezel_repo # App repository

def btn_unhandled(pressed):
	display.flush(display.FLAG_LUT_FASTEST)

def btn_exit(pressed):
	if pressed:
		system.launcher()

def btn_update(pressed):
	if pressed:
		repo.update()
		system.start("dashboard.installer", True)

def show_categories(pressed = True, fromAppInstall = False):
	if not pressed:
		return
	ugfx.input_attach(ugfx.BTN_BACK, btn_unhandled)
	ugfx.input_attach(ugfx.BTN_OK, btn_unhandled)
	ugfx.input_attach(ugfx.JOY_UP, btn_unhandled)
	ugfx.input_attach(ugfx.JOY_DOWN, btn_unhandled)
	ugfx.input_attach(ugfx.JOY_LEFT, btn_unhandled)
	ugfx.input_attach(ugfx.JOY_RIGHT, btn_unhandled)
	ugfx.clear(ugfx.WHITE)
	selectBox.clear()
	for category in repo.categories:
		selectBox.add_item(category["name"])
	selectBox.enabled(True)
	selectBox.visible(True)
	ugfx.input_attach(ugfx.BTN_BACK, btn_exit)
	ugfx.input_attach(ugfx.BTN_OK, show_category)
	ugfx.input_attach(ugfx.JOY_UP, btn_unhandled)
	ugfx.input_attach(ugfx.JOY_DOWN, btn_unhandled)
	ugfx.input_attach(ugfx.JOY_LEFT, btn_update)
	ugfx.input_attach(ugfx.JOY_RIGHT, btn_unhandled)
	display.flush(display.FLAG_LUT_NORMAL)

def show_category(pressed=True, fromAppInstall = False):
	global lastCategory
	if not pressed:
		return
	ugfx.input_attach(ugfx.BTN_BACK, btn_unhandled)
	ugfx.input_attach(ugfx.BTN_OK, btn_unhandled)
	ugfx.input_attach(ugfx.JOY_UP, btn_unhandled)
	ugfx.input_attach(ugfx.JOY_DOWN, btn_unhandled)
	ugfx.input_attach(ugfx.JOY_LEFT, btn_unhandled)
	ugfx.input_attach(ugfx.JOY_RIGHT, btn_unhandled)
	ugfx.clear(ugfx.WHITE)
	global category
	if not fromAppInstall:
		lastCategory = selectBox.selected_index()
	slug = repo.categories[lastCategory]["slug"]
	easydraw.messageCentered("Loading category...\n"+slug, True, "/media/busy.png")
	display.drawFill()
	selectBox.clear()
	try:
		category = repo.getCategory(slug)
		gc.collect()
		for package in category:
			selectBox.add_item(package["name"])
		selectBox.visible(True)
		selectBox.enabled(True)
		#Input handling
		ugfx.input_attach(ugfx.BTN_OK, install_app)
		ugfx.input_attach(ugfx.BTN_BACK, show_categories)
		ugfx.input_attach(ugfx.JOY_UP, btn_unhandled)
		ugfx.input_attach(ugfx.JOY_DOWN, btn_unhandled)
		ugfx.input_attach(ugfx.JOY_LEFT, btn_unhandled)
		ugfx.input_attach(ugfx.JOY_RIGHT, btn_unhandled)
		#Flush screen
		display.flush(display.FLAG_LUT_NORMAL)
	except BaseException as e:
		sys.print_exception(e)
		easydraw.messageCentered("An error occured!", False, "/media/alert.png")
		display.drawFill()
		time.sleep(1)
		show_categories()

def install_app(pressed=True):
	global category
	if pressed:
		slug = category[selectBox.selected_index()]["slug"]
		#Input handling
		ugfx.input_attach(ugfx.BTN_BACK, btn_unhandled)
		ugfx.input_attach(ugfx.BTN_OK, btn_unhandled)
		ugfx.input_attach(ugfx.JOY_UP, btn_unhandled)
		ugfx.input_attach(ugfx.JOY_DOWN, btn_unhandled)
		ugfx.input_attach(ugfx.JOY_LEFT, btn_unhandled)
		ugfx.input_attach(ugfx.JOY_RIGHT, btn_unhandled)
		category = []
		selectBox.clear()
		easydraw.messageCentered("Installing...\n"+slug, True, "/media/busy.png")
		with open("/cache/installList", "w") as f:
			f.write(slug)
		system.start("dashboard._installer_exec")

# ----

orientation.default()
selectBox = ugfx.List(0,0,ugfx.width(),ugfx.height())
repo = woezel_repo
lastCategory = 0

easydraw.messageCentered("Loading...", True, "/media/busy.png")
display.drawFill()
if not repo.load():
	if not repo.update():
		if repo.lastUpdate==0:
			easydraw.messageCentered("Repository not available!", False, "/media/alert.png")
			display.drawFill()
			system.launcher()

show_categories()
