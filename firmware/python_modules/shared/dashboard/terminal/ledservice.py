import term, orientation, system, time, uos, json, machine

system.serialWarning()
term.header(True, "Services")
print("Loading...")
apps = uos.listdir('/lib')
services = []
for app in apps:
	if "ledsrv.py" in uos.listdir('/lib/'+app):
		services.append(app)

current = machine.nvs_getstr('splash', 'ledApp')

while True:
	options = []
	for service in services:
		title = service
		if service == current:
			title += " [Enabled]"
		options.append(title)
	options.append(" None (disable LED services)")
	options.append("< Exit")

	selected = term.menu("Services", options, 0, "")
	if selected > len(services):
		system.home(True)
	if selected == len(services):
		current = None
		try:
			machine.nvs_erase('splash', 'ledApp')
		except:
			pass
		term.header(True, "Services")
		print("LED services disabled")
		time.sleep(1)
	else:
		current = services[selected]
		machine.nvs_setstr('splash', 'ledApp', current)
		term.header(True, "Services")
		print("Selected "+current)
		time.sleep(1)
