import term, orientation, system, time, uos, json

system.serialWarning()
term.header(True, "Services")
print("Loading...")
apps = uos.listdir('/lib')
services = []
for app in apps:
	if "srv.py" in uos.listdir('/lib/'+app):
		services.append(app)

def load():
	global data
	f = open("/services.json", "r")
	data = json.loads(f.read())
	f.close()

def save():
	global data
	f = open("/services.json", "w")
	f.write(json.dumps(data))
	f.close()

load()

while True:
	options = []
	for service in services:
		title = service
		if service in data['apps']:
			title += " [Enabled]"
		options.append(title)
	options.append("< Exit")

	selected = term.menu("Services", options, 0, "")
	print("Selected", selected)
	if selected == len(services):
		system.home(True)
	if services[selected] in data['apps']:
		data['apps'].remove(services[selected])
		term.header(True, "Services")
		print(services[selected]+" has been disabled.")
		save()
		time.sleep(1)
	else:
		data['apps'].append(services[selected])
		print(services[selected]+" has been enabled.")
		save()
		time.sleep(1)
