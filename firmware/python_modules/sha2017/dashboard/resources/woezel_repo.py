import time, machine, gc, easydraw, term, uos, json, urequests, gc, sys, wifi

path = "/woezel_packages"
categories = []
lastUpdate = 0

try:
	uos.mkdir(path)
except:
	pass

def setPath(newPath="/woezel_packages"):
	global path
	path = newPath
	categories = []
	lastUpdate = 0
	try:
		uos.mkdir(path)
	except:
		pass

def _showProgress(msg, error=False, icon_wifi=False):
	term.header(True, "Installer")
	print(msg)
	if error:
		easydraw.messageCentered("ERROR\n\n"+msg, True, "/media/alert.png")
	else:
		if icon_wifi:
			easydraw.messageCentered("PLEASE WAIT\n\n"+msg, True, "/media/wifi.png")
		else:
			easydraw.messageCentered("PLEASE WAIT\n\n"+msg, True, "/media/busy.png")

def update():
	global path, categories, lastUpdate
	if not wifi.status():
		_showProgress("Connecting to WiFi...", False, True)
		wifi.connect()
		if not wifi.wait():
			_showProgress("Failed to connect to WiFi.", True, False)
			return False
	_showProgress("Downloading categories...")
	try:
		request = urequests.get("https://badge.team/eggs/categories/json", timeout=30)
		_showProgress("Saving categories...")
		categories_file = open(path+'/categories.json', 'w')
		categories_file.write(request.text)
		categories_file.close()
		_showProgress("Parsing categories...")
		categories = request.json()
		for category in categories:
			gc.collect()
			slug = category["slug"]
			_showProgress("Downloading '"+category["name"]+"'...")
			f = urequests.get("https://badge.team/basket/hackerhotel2019/category/%s/json" % slug, timeout=30)
			f_file = open(path+'/'+slug+'.json', 'w')
			f_file.write(f.text)
			f_file.close()
		lastUpdate = int(time.time())
		f = open(path+"/lastUpdate", 'w')
		f.write(str(lastUpdate))
		f.close()
		_showProgress("Done!")
		gc.collect()
		return True
	except BaseException as e:
		sys.print_exception(e)
		_showProgress("Failed!", True)
		gc.collect()
	return False

def load():
	global path, categories, lastUpdate
	try:
		f = open(path+"/lastUpdate", 'r')
		data = f.read()
		f.close()
		#print("Last update at",data)
		lastUpdate = int(data)
		f = open(path+"/categories.json")
		categories = json.loads(f.read())
		f.close()
		gc.collect()
		if (lastUpdate + 900) < int(time.time()):
			print("Too old", lastUpdate + 900, "<", int(time.time()))
			time.sleep(2)
			return False
		return True
	except BaseException as e:
		sys.print_exception(e)
		time.sleep(2)
		return False

def getCategory(slug):
	global path
	f = open(path+"/"+slug+".json")
	data = json.loads(f.read())
	f.close()
	gc.collect()
	return data
