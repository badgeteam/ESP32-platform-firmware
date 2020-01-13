import time, display, _thread, wifi, term, _buttons, machine, binascii, system
from umqtt.simple import MQTTClient

# System settings
mac_address   = wifi._STA_IF.config("mac")
mqtt_hostname = "decennium-{}".format(binascii.hexlify(mac_address))
mqtt_server   = machine.nvs_getstr("hsnl", "mqtt_server") or "hoera10jaar.revspace.nl"
ota_allowed   = machine.nvs_get_u8("hsnl", "ota_allowed") or 1

# Global state variables
wifi_ap = False

# Hackerspaces
spaces = [
	{ "red": (0,0), "grn": (3,0), "channel": "leeuwarden", "state": 0 },
	{ "red": (0,1), "grn": (3,1), "channel": "amsterdam",  "state": 0 },
	{ "red": (0,2), "grn": (3,2), "channel": "utrecht",    "state": 0 },
	{ "red": (0,3), "grn": (3,3), "channel": "denhaag",    "state": 0 },
	{ "red": (0,4), "grn": (3,4), "channel": "rotterdam",  "state": 0 },
	{ "red": (1,0), "grn": (4,0), "channel": "zwolle",     "state": 0 },
	{ "red": (1,1), "grn": (4,1), "channel": "amersfoort", "state": 0 },
	{ "red": (1,2), "grn": (4,2), "channel": "arnhem",     "state": 0 },
	{ "red": (1,3), "grn": (4,3), "channel": "wageningen", "state": 0 },
	{ "red": (1,4), "grn": (4,4), "channel": "eindhoven",  "state": 0 },
	{ "red": (2,1), "grn": (5,1), "channel": "enschede",   "state": 0 },
	{ "red": (2,2), "grn": (5,2), "channel": "nijmegen",   "state": 0 },
	{ "red": (2,3), "grn": (5,3), "channel": "venlo",      "state": 0 },
	{ "red": (2,4), "grn": (5,4), "channel": "heerlen",    "state": 0 },
]

# LED matrix

def color(color):
	return color | color << 8 | color << 16

def ledThread():
	global spaces
	while 1:
		for i in range(len(spaces)):
			# Position
			xR, yR = spaces[i]["red"]
			xG, yG = spaces[i]["grn"]
			state  = spaces[i]["state"]
			
			# Get LEDs
			R = display.getPixel(xR, yR) >> 16
			G = display.getPixel(xG, yG) >> 16
			
			red_brightness = 0x15
			if state == 3:
				red_brightness = 0x30
			
			tgtR  = ((state == 1) or (state == 3)) * red_brightness
			tgtG  = ((state == 2) or (state == 3)) * 0x10
			
			# Set LEDs
			if R < tgtR:
				R = R + 1
			elif R > tgtR:
				R = R - 1
			
			if G < tgtG:
				G = G + 1
			elif G > tgtG:
				G = G - 1

			display.drawPixel(xR,yR,color(R))
			display.drawPixel(xG,yG,color(G))
		display.flush()
		time.sleep_ms(5)

# Space state
def setAll(state):
	for i in range(len(spaces)):
		spaces[i]['state'] = state

# Button
def buttonHandler(pressed):
	global wifi_ap
	if pressed:
		#wifi_ap = True
		setAll(0)
		system.start("dashboard.webconfig")

# MQTT
def mqttHandler(topic, payload):
	topic = topic.decode("ascii")
	payload = payload.decode("ascii")
	if topic.startswith("hoera10jaar/"):
		topic = topic[12:]
		found = False
		for i in range(len(spaces)):
			if topic == spaces[i]["channel"]:
				found = True
				if payload == "red":
					spaces[i]["state"] = 1
				elif payload == "green":
					spaces[i]["state"] = 2
				elif payload == "yellow":
					spaces[i]["state"] = 3
				else:
					spaces[i]["state"] = 0
		if not found:
			print("Received message about the status of an unknown space", topic, payload)
	else:
		print("Received message on unexpected topic", topic, payload)

# Connection
def connectToWiFi():
	global wifi_ap
	connecting = True
	while connecting:
		print("Connecting to WiFi...")
		wifi.connect()
		for i in range(10):
			if i & 1:
				setAll(0)
			else:
				setAll(3)
			time.sleep(0.5)
			if wifi.status() or wifi_ap:
				connecting = False
				break
		if not wifi.status():
			setAll(1)
			time.sleep(2)
			setAll(0)
			time.sleep(2)
	#setAll(2)
	#time.sleep(1)
	setAll(0)
	return wifi.status()

def main():
	try:
		global mqtt_hostname, mqtt_server, wifi_ap
		_thread.start_new_thread("led-animation", ledThread, ())
		_buttons.register(0)
		_buttons.attach(0, buttonHandler)
		mqtt = MQTTClient(mqtt_hostname, mqtt_server)
		mqtt.set_callback(mqttHandler)
		
		while True:
			if wifi_ap:
				mqtt.disconnect()
				break
			if not wifi.status():
				connectToWiFi()
				mqtt.connect()
				mqtt.subscribe(b"hoera10jaar/+")
			else:
				mqtt.check_msg()
			time.sleep(0.1)
	except BaseException as e:
		print("MAIN THREAD CRASHED")
		print(e)
		time.sleep(2)
		system.home()

_thread.start_new_thread("main-app", main, ())
