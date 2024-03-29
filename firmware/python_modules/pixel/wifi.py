import network, time, machine, consts

_STA_IF = network.WLAN(network.STA_IF)
_AP_IF  = network.WLAN(network.AP_IF)

_DEFAULT_TIMEOUT  = machine.nvs_getint("system", "wifi.timeout") or 20
_DEFAULT_SSID     = machine.nvs_getstr("system", "wifi.ssid")
_DEFAULT_PASSWORD = machine.nvs_getstr("system", "wifi.password")

if not _DEFAULT_SSID:
	_DEFAULT_SSID     = consts.WIFI_SSID
	_DEFAULT_PASSWORD = consts.WIFI_PASSWORD

# STATION MODE
# ------------

def connect(*args):
	'''
	Connect to a WiFi network
	:param ssid: optional, ssid of network to connect to
	:param password: optional, password of network to connect to
	'''
	_STA_IF.active(True)
	if len(args) == 0:
		if _DEFAULT_PASSWORD:
			_STA_IF.connect(_DEFAULT_SSID, _DEFAULT_PASSWORD)
		else:
			_STA_IF.connect(_DEFAULT_SSID)
	elif len(args) == 1:
		_STA_IF.connect(args[0])
	elif len(args) == 2:
		_STA_IF.connect(args[0], args[1])
	else:
		raise Exception('Expected either 0 (default network), 1 (ssid) or 2 (ssid, password) parameters.')

def disconnect():
	'''
	Disconnect from a WiFi network
	'''
	_STA_IF.disconnect()

def status():
	'''
	Connection status
	:return: boolean, connected
	'''
	return _STA_IF.isconnected()

def wait(duration=_DEFAULT_TIMEOUT, UNUSED_LEGACY_PARAMETER=None):
	'''
	Wait until connection has been made
	:return: boolean, connected
	'''
	t = duration
	while not status():
		if t <= 0:
			break
		# if t % 2 == 0:
		# 	connect()
		t -= 1
		time.sleep(1)
	return status()

def ntp(onlyIfNeeded=True, server='pool.ntp.org'):
	'''
	Synchronize the system clock with NTP
	:return: boolean, synchronized
	'''
	if onlyIfNeeded and time.time() > 1482192000:
		return True #RTC is already set, sync not needed
	rtc = machine.RTC()
	if not status():
		return False # Not connected to a WiFi network
	return rtc.ntp_sync(server)

def scan():
	'''
	Scan for WiFi networks
	:return: list, wifi networks [SSID, BSSID, CHANNEL, RSSI, AUTHMODE1, AUTHMODE2, HIDDEN]
	'''
	return _STA_IF.scan()

# ACCESS POINT MODE
# -----------------
def accesspoint_start(ssid, password=None):
	'''
	Create a WiFi access point
	:param ssid: SSID of the network
	:param password: Password of the network (optional)
	'''
	if password:
		_AP_IF.config(essid=ssid, authmode=network.AUTH_WPA2_PSK, password=password)
	else:
		_AP_IF.config(essid=ssid, authmode=network.AUTH_OPEN)
	_AP_IF.active(True)

def accesspoint_status():
	return _AP_IF.active()

def accesspoint_stop():
	_AP_IF.active(False)
