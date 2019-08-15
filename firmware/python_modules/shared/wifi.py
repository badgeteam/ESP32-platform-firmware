import network, time, machine, consts

sta_if = network.WLAN(network.STA_IF)

timeout         = machine.nvs_getint("system", "wifi.timeout") or 10
defaultSsid     = machine.nvs_getstr("system", "wifi.ssid")
defaultPassword = machine.nvs_getstr("system", "wifi.password")

if not defaultSsid:
	defaultSsid     = consts.WIFI_SSID
	defaultPassword = consts.WIFI_PASSWORD

def connect(ssid=defaultSsid, password=defaultPassword):
	sta_if.active(True)
	if ssid and password:
		sta_if.connect(ssid, password)
	elif ssid:
		sta_if.connect(ssid)

def disconnect():
	sta_if.disconnect()

def status():
	return sta_if.isconnected()

def wait(duration=timeout, UNUSED_LEGACY_PARAMETER=None):
	t = duration*10
	while not status():
		if t <= 0:
			break
		t -= 1
		time.sleep(0.1)
	return status()

def ntp(onlyIfNeeded=True):
	if onlyIfNeeded and time.time() > 1482192000:
		return True
	rtc = machine.RTC()
	if not status():
		connect()
		if not wait():
			return False

	return rtc.ntp_sync('pool.ntp.org')
