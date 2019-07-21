import network, time, machine, consts

sta_if = network.WLAN(network.STA_IF)

defaultSsid = machine.nvs_getstr("system", "wifi.ssid") or consts.WIFI_SSID
defaultPassword = machine.nvs_getstr("system", "wifi.password")

# Password is empty for unencrypted networks, so a simple or statement won't do
if defaultPassword is None:
	defaultPassword = consts.WIFI_PASSWORD

timeout = machine.nvs_getint("system", "wifi.timeout") or 10

def status():
	return sta_if.isconnected()

def connect(ssid=defaultSsid, password=defaultPassword):
	global sta_if
	if not ssid:
		return status()
	sta_if.active(True)
	print("WiFi connect to",ssid,password)
	if password:
		sta_if.connect(ssid, password)
	else:
		sta_if.connect(ssid)

def disconnect():
	global sta_if
	sta_if.disconnect()

def ntp(onlyIfNeeded=True):
	if onlyIfNeeded and time.time() > 1482192000:
		return True
	from machine import RTC
	rtc = RTC()
	if not status():
		connect()
		if not wait():
			return False

	return rtc.ntp_sync('pool.ntp.org')

def wait(duration=timeout, showStatus=False):
	t = duration*10
	while not status():
		if t <= 0:
			break
		t -= 1
		time.sleep(0.1)
	return status()
