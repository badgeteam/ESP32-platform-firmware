import easydraw, network, machine, system, keyboard, ugfx

easydraw.messageCentered("Scanning...", True, "/media/wifi.png")
sta_if = network.WLAN(network.STA_IF); sta_if.active(True)
scanResults = sta_if.scan()

ssidList = []
for AP in scanResults:
	ssidList.append(AP[0])
ssidSet = set(ssidList)

options = ugfx.List(0,0,ugfx.width(),ugfx.height())

for ssid in ssidSet:
	try:
		ssidStr = ssid.decode("ascii")
		options.add_item(ssidStr)
	except:
		pass

chosenSsid = ""
def connectClick(pushed):
	global chosenSsid
	if pushed:
		selected = options.selected_text().encode()
		
		ssidType = scanResults[ssidList.index(selected)][4]
		if ssidType == 5:
			easydraw.messageCentered("WPA Enterprise is not supported yet.", True, "/media/alert.png")
			system.reboot()
		
		chosenSsid = selected
		if ssidType == 0:
			passInputDone(None)
		else:
			keyboard.show("Password","",passInputDone)

def passInputDone(password):
	global chosenSsid
	machine.nvs_setstr("system", "wifi.ssid", chosenSsid)
	if password:
		machine.nvs_setstr("system", "wifi.password", password)
	else:
		try:
			machine.nvs_erase("system", "wifi.password")
		except:
			pass
	easydraw.messageCentered("Settings stored!", True, "/media/ok.png")
	system.launcher()

def exitApp(pressed):
	if pressed:
		system.launcher()

ugfx.input_attach(ugfx.BTN_A, connectClick)
ugfx.input_attach(ugfx.BTN_B, exitApp)
try:
	ugfx.input_attach(ugfx.BTN_START, exitApp)
except:
	pass
ugfx.input_attach(ugfx.JOY_UP, lambda pushed: ugfx.flush() if pushed else 0)
ugfx.input_attach(ugfx.JOY_DOWN, lambda pushed: ugfx.flush() if pushed else 0)
ugfx.set_lut(ugfx.LUT_FASTER)
ugfx.flush()
