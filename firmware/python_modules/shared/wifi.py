import network, time, machine, consts

sta_if = network.WLAN(network.STA_IF)

defaultSsid = machine.nvs_getstr("system", "wifi.ssid") or consts.WIFI_SSID
defaultPassword = machine.nvs_getstr("system", "wifi.password") or consts.WIFI_PASSWORD
timeout = machine.nvs_get_u16("system", "wifi.timeout")

if not defaultSsid:
	print("FIXME: GET DEFAULT SSID")
	defaultSsid = ""

if not defaultPassword:
	print("FIXME: GET DEFAULT PASSWORD")
	defaultSsid = ""

if timeout == None:
	timeout = 10

def status():
	global sta_if
	if sta_if == None:
		return False
	return sta_if.isconnected()

def connect(ssid=defaultSsid, password=defaultPassword):
	global sta_if
	sta_if.active(True)
	if password:
		sta_if.connect(ssid, password)
	else:
		sta_if.connect(ssid)

	if not status():
		for i in range(0,3):
			time.sleep(1)
			if status():
				break

	return status()

def disconnect():
	global sta_if
	sta_if.disconnect()

def ntp(onlyIfNeeded=True):
	if onlyIfNeeded and time.time() > 1482192000:
		return True
	from machine import RTC
	rtc = RTC()
	if not status():
		return False
	return rtc.ntp_sync('pool.ntp.org')

def wait(duration=timeout, showStatus=False):
	t = timeout*10
	while not status():
		if timeout <= 0:
			break
		timeout -= 1
		time.sleep(0.1)
