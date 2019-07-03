import ugfx, badge, network, gc, time, urequests, appglue, sys, easydraw, version

def connectWiFi():
    nw = network.WLAN(network.STA_IF)
    if not nw.isconnected():
        nw.active(True)
        ssid = badge.nvs_get_str('badge', 'wifi.ssid', '')
        password = badge.nvs_get_str('badge', 'wifi.password', '')
        nw.connect(ssid, password) if password else nw.connect(ssid)

        easydraw.msg("Connecting to wifi...", "Loading...", True)

        timeout = 150
        while not nw.isconnected():
            time.sleep(0.1)
            timeout = timeout - 1
            if (timeout<1):
                easydraw.msg("Timeout :(")
                nw.active(True)
                return False
    return True

def show_description(active):
	if active:
		easydraw.msg(packages[options.selected_index()]["description"],"Details",True)
	else:
		options.visible(0)
		options.visible(1)
	ugfx.flush()

def select_category(active):
	if active:
		global categories
		global options
		index = options.selected_index()
		if categories[index]["eggs"] > 0:
			category = categories[index]["slug"]
			list_apps(category)

def dummy_button_cb(pressed):
	print("No function attached to this button! (",pressed,")")
	ugfx.flush()

def list_apps(slug):
	global options
	global packages

	ugfx.input_attach(ugfx.JOY_UP, dummy_button_cb)
	ugfx.input_attach(ugfx.JOY_DOWN, dummy_button_cb)
	ugfx.input_attach(ugfx.BTN_B, dummy_button_cb)
	ugfx.input_attach(ugfx.BTN_START, dummy_button_cb)

	while options.count() > 0:
		options.remove_item(0)
	easydraw.msg("Loading list...","Loading...",True)
	ugfx.flush(ugfx.LUT_FULL)

	try:
		f = urequests.get("https://badge.disobey.fi/eggs/category/%s/json" % slug, timeout=30)
		try:
			packages = f.json()
		finally:
			f.close()
	except BaseException as e:
		print("[Installer] Failed to download list of eggs:")
		sys.print_exception(e)
		easydraw.msg("Failed :(")
		ugfx.flush(ugfx.LUT_FULL)
		list_categories()
		gc.collect()
		return

	for package in packages:
		options.add_item("%s rev. %s" % (package["name"], package["revision"]))
	options.selected_index(0)

	ugfx.input_attach(ugfx.JOY_RIGHT, show_description)
	ugfx.input_attach(ugfx.BTN_START, install_app)
	ugfx.input_attach(ugfx.BTN_B, lambda pushed: list_categories() if pushed else False)
	gc.collect()

def start_categories(pushed):
	if pushed:
		list_categories()

def start_app(pushed):
	if pushed:
		global selected_app
		appglue.start_app(selected_app)

def install_app(active):
	if active:
		global options
		global packages
		global selected_app

		index = options.selected_index()

		ugfx.input_attach(ugfx.JOY_UP, dummy_button_cb)
		ugfx.input_attach(ugfx.JOY_DOWN, dummy_button_cb)
		ugfx.input_attach(ugfx.BTN_B, dummy_button_cb)
		ugfx.input_attach(ugfx.BTN_START, dummy_button_cb)

		easydraw.msg(packages[index]["name"], "Installing...", True)

		latest = False
		import woezel
		selected_app =  packages[index]["slug"]
		try:
			woezel.install(selected_app)
		except woezel.LatestInstalledError:
			latest = True
		except:
			easydraw.msg("Failed :(")
			time.sleep(4)
			list_categories()
			return

		if latest:
			easydraw.msg("Already installed!")
		else:
			easydraw.msg("Installed!")
		easydraw.msg(packages[index]["name"])
		easydraw.msg("[BACK]                    [RUN]")
		ugfx.input_attach(ugfx.BTN_START, start_app)
		ugfx.input_attach(ugfx.BTN_B, start_categories)
		gc.collect()

def list_categories():
	global options
	global categories

	try:
		categories
	except:
		ugfx.input_init()
		easydraw.msg('Getting categories',"Loading...",True)
		try:
			f = urequests.get("https://badge.disobey.fi/eggs/categories/json", timeout=30)
			categories = f.json()
		except:
			easydraw.msg('Failed :(')
			appglue.start_app('launcher', False)

			f.close()
		easydraw.msg('Success :)')
		options = ugfx.List(0,0,int(ugfx.width()),ugfx.height())



	ugfx.input_attach(ugfx.JOY_UP, dummy_button_cb)
	ugfx.input_attach(ugfx.JOY_DOWN, dummy_button_cb)
	ugfx.input_attach(ugfx.BTN_START, select_category)
	ugfx.input_attach(ugfx.BTN_B, lambda pushed: appglue.start_app("launcher", False) if pushed else False)

	ugfx.clear(ugfx.WHITE)
	ugfx.flush()

	while options.count() > 0:
		options.remove_item(0)
	for category in categories:
		options.add_item("%s (%d) >" % (category["name"], category["eggs"]))
	options.selected_index(0)
	ugfx.flush(ugfx.LUT_FULL)
	gc.collect()


if not connectWiFi():
	easydraw.msg('WiFi failure :(')
	appglue.start_app('launcher', False)
else:
	list_categories()
