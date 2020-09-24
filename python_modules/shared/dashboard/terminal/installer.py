import dashboard.resources.woezel_repo as woezel_repo, woezel, term, system, time, wifi, sys, gc

repo = woezel_repo

def showMessage(msg, error=False):
	term.header(True, "Installer")
	print(msg)

def show_categories():
	system.serialWarning()
	while True:
		opt = []
		for category in repo.categories:
			opt.append(category["name"])
		opt.append("< Back to launcher")
		sel = term.menu("Installer - Categories", opt)
		if sel == len(repo.categories):
			system.launcher(True)
		opt = []
		gc.collect()
		show_category(repo.categories[sel]["slug"], repo.categories[sel]["name"])

def show_category(slug, name):
	system.serialWarning()
	showMessage("Loading "+slug+"...")
	try:
		category = repo.getCategory(slug)
	except BaseException as e:
		showMessage("Failed to open category "+slug+"!", True)
		sys.print_exception(e)
		time.sleep(1)
		return
	gc.collect()
	try:
		opt = []
		for package in category:
			opt.append("%s rev. %s" % (package["name"], package["revision"]))
		opt.append("< Back to categories")
		sel = term.menu("Installer - "+name, opt)
		if sel == len(opt)-1:
			return
		slug  = category[sel]["slug"]
		category = None
		gc.collect()
		install_app(slug)
	except BaseException as e:
		sys.print_exception(e)
		showMessage(e, True)
		time.sleep(1)
		return

def install_app(slug):
	system.serialWarning()
	if not wifi.status():
		wifi.connect()
		wifi.wait()
		if not wifi.status():
			showMessage("Unable to connect to WiFi.")
			time.sleep(2)
			return
	showMessage("Installing "+slug+"...")
	try:
		gc.collect()
		gc.collect()
		gc.collect()
		woezel.install(slug)
		showMessage(slug+" has been installed!")
	except woezel.LatestInstalledError:
		showMessage("Latest version is already installed.")
	except:
		showMessage("Failed to install "+slug+"!")
	time.sleep(2)

showMessage("Loading categories...")
if not repo.load():
	if not repo.update():
		if repo.lastUpdate==0:
			showMessage("Failed to load repository. Returning to launcher...")
			time.sleep(2)
			system.launcher()

show_categories()
