import network, term, appglue, sys, badge, version, time,json, gc, urequests, easydraw

easydraw.msg("This app can only be controlled using the USB-serial connection.", "Notice", True)

#Teminal installer
#V1 Kliment


#categories_fake=json.loads('[{"name":"stupid","eggs":1,"slug":"stupid"},{"name":"trolley","eggs":3,"slug":"trolley"},{"name":"empty","eggs":0,"slug":"empty"}]')
#eggs_fake=None#json.loads('[{"name":"app1","description":"This is app 1","slug":"app1"},{"name":"app2","description":"This is app 2","slug":"app2"}]')

#categories=None#categories_fake

def connectWiFi():
    nw = network.WLAN(network.STA_IF)
    if not nw.isconnected():
        nw.active(True)
        ssid = badge.nvs_get_str('badge', 'wifi.ssid', '')
        password = badge.nvs_get_str('badge', 'wifi.password', '')
        nw.connect(ssid, password) if password else nw.connect(ssid)

        print("Connecting to '"+ssid+"'...")

        timeout = 150
        while not nw.isconnected():
            time.sleep(0.1)
            timeout = timeout - 1
            if (timeout<1):
                print("Timeout while connecting!")
                nw.active(True)
                return False
    return True

def main():
	items=["Install apps", "Update repository", "Return to main menu"]
	callbacks = [list_categories, update_repo, home]
	callbacks[term.menu("Installer", items)]()

def home():
	appglue.home()

def update_repo():
	global categories
	print("Fetching categories")
	try:
		f = urequests.get(version.hatcheryurl+"/eggs/categories/json", timeout=30)
		categories = f.json()
	except:
		home()
		f.close()
	print('Done!')


def list_apps(slug):
	print("Downloading list of eggs...")
	packages=None
	try:
		f = urequests.get(version.hatcheryurl+"/basket/hackerhotel2019/category/%s/json" % slug, timeout=30)
		try:
			packages = f.json()
		finally:
			f.close()
	except BaseException as e:
		gc.collect()
		return None
	return packages
	gc.collect()


def list_categories():
	global categories
	while(1):
		try:
			categories
		except:
			update_repo()
		cat_list = []

		for category in categories:
			if category['eggs']>0:
				cat_list.append(category["name"])
		cat_list.append("< Back")
		option = term.menu("Installer - Select a category", cat_list)
		if option != len(cat_list)-1:
			catsel=None
			for category in categories:
				if category['name']==cat_list[option]:
					catsel=category
					break
			if catsel is not None:
				open_category(catsel)
		else:
			return
		gc.collect()


def open_category(category):
	while 1:
		apps_in_cat=list_apps(category['slug'])
		if apps_in_cat is None:
			return
		app_list=[]
		for app in apps_in_cat:
			app_list.append(app["name"])
		app_list.append("< Back")
		option = term.menu("Installer - Select an app", app_list)
		if option != len(app_list)-1:
			appsel=None
			for app in apps_in_cat:
				if app['name']==app_list[option]:
					appsel=app
					break
			if appsel is not None:
				show_app(app,category)
		else:
			return
		gc.collect()


def show_app(app,category):
	term.empty_lines()
	term.header(True, "Installer - %s"%app['name'])
	items=["Install app %s"%app['name'], "Go back"]
	option=term.menu("Installer %s %s"%(app['name'],app['description']), items)
	if option==0:
		install_app(app)
	else:
		return
	gc.collect()

def install_app(app):
	latest = False
	import woezel
	selected_app =  app["slug"]
	try:
		woezel.install(selected_app)
	except woezel.LatestInstalledError:
		latest = True
	except:
		print('Failed to install!')
		time.sleep(2)
		return
	gc.collect()
	header="Installer"
	if latest:
		header+=(" - You already have the latest version of %s"%app['name'])
	else:
		header+=(" - app %s installed"%app[name])
	items=["Run %s"%app['name'], "Go back"]
	option=term.menu(header, items)
	if option==0:
		appglue.start_app(app['slug'])
	else:
		return
	gc.collect()



if not connectWiFi():
		print('No network. Returning to launcher :(')
		home()
while True:
	main()
