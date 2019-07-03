import network, badge, time

sta_if = None
defaultSsid = badge.nvs_get_str('badge', 'wifi.ssid', 'hackerhotel-insecure')
defaultPassword = badge.nvs_get_str('badge', 'wifi.password')
timeout = badge.nvs_get_str('badge', 'wifi.timeout', 10)

def status():
	global sta_if
	if sta_if == None:
		return False
	return sta_if.isconnected()

def connect(ssid=defaultSsid, password=defaultPassword):
	init()
	sta_if.active(True)
	if password:
		sta_if.connect(ssid, password)
	else:
		sta_if.connect(ssid)

def disconnect():
	sta_if.disconnect()

def init():
	global sta_if
	if sta_if == None:
		sta_if = network.WLAN(network.STA_IF)

def wait(duration=timeout, showStatus=False):
	if showStatus:
		import easydraw, term
		term.header(True, "Connecting...")
		print("Connecting to WiFi...")
		easydraw.messageCentered("Connecting...", True, "/media/wifi.png")
	while not status():
		time.sleep(1)
		duration -= 1
		if duration < 0:
			if showStatus:
				term.header(True, "Failed!")
				print("Could not connect to WiFi.")
				easydraw.messageCentered("Failed!", True, "/media/alert.png")
			return False
	if showStatus:
		term.header(True, "Connected!")
		print("Connected to WiFi.")
		easydraw.messageCentered("Connected!", True, "/media/ok.png")
	return True

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
