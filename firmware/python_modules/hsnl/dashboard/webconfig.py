import socket, network, wifi,  time, display, system, _buttons, binascii, machine

leds = [
	{ "red": (0,0), "grn": (3,0) },
	{ "red": (0,1), "grn": (3,1) },
	{ "red": (0,2), "grn": (3,2) },
	{ "red": (0,3), "grn": (3,3) },
	{ "red": (0,4), "grn": (3,4) },
	{ "red": (1,0), "grn": (4,0) },
	{ "red": (1,1), "grn": (4,1) },
	{ "red": (1,2), "grn": (4,2) },
	{ "red": (1,3), "grn": (4,3) },
	{ "red": (1,4), "grn": (4,4) },
	{ "red": (2,1), "grn": (5,1) },
	{ "red": (2,2), "grn": (5,2) },
	{ "red": (2,3), "grn": (5,3) },
	{ "red": (2,4), "grn": (5,4) },
]

mac_address   = wifi._STA_IF.config("mac")
ap_ssid       = "decennium-{}".format(binascii.hexlify(mac_address).decode("ascii"))
ap_password   = machine.nvs_getstr("hsnl", "ap_password") or None

wifiList = []

def start_ap():
	global ap_ssid, ap_password
	ap_if = network.WLAN(network.AP_IF)
	ap_if.active(True)
	if ap_password:
		ap_if.config(essid=ap_ssid, authmode=network.AUTH_WPA2_PSK, password=ap_password)
	else:
		ap_if.config(essid=ap_ssid, authmode=network.AUTH_OPEN)

def stop_ap():
	ap_if = network.WLAN(network.AP_IF)
	ap_if.active(False)

def dnsserver():
	global dns_socket
	ap_if = network.WLAN(network.AP_IF)
	dns_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	dns_socket.bind((ap_if.ifconfig()[0], 53))
	http_socket.setblocking(False)

def handle_dns():
	global dns_socket
	ap_if = network.WLAN(network.AP_IF)
	try:
		data, addr = dns_socket.recvfrom(4096)
		#print('[DNS] Client connected from', addr)
		
		if len(data) < 13:
			print('[DNS] Ignored request 1')
			return
		
		domain = ''
		tipo = (data[2] >> 3) & 15  # Opcode bits
		if tipo == 0:  # Standard query
			ini = 12
			lon = data[ini]
			while lon != 0:
				domain += data[ini + 1:ini + lon + 1].decode('utf-8') + '.'
				ini += lon + 1
				lon = data[ini]
			packet = data[:2] + b'\x81\x80'
			packet += data[4:6] + data[4:6] + b'\x00\x00\x00\x00'  # Questions and Answers Counts
			packet += data[12:]  # Original Domain Name Question
			packet += b'\xC0\x0C'  # Pointer to domain name
			packet += b'\x00\x01\x00\x01\x00\x00\x00\x3C\x00\x04'  # Response type, ttl and resource data length -> 4 bytes
			packet += bytes(map(int, ap_if.ifconfig()[0].split('.')))  # 4bytes of IP
			dns_socket.sendto(packet, addr)
		else:
			print('[DNS] Ignored request 2')
	except:
		pass

def webserver():
	global http_socket
	addr = socket.getaddrinfo('0.0.0.0', 80)[0][-1]
	http_socket = socket.socket()
	http_socket.bind(addr)
	http_socket.listen(True)
	http_socket.setblocking(False)

def handle_web():
	global http_socket
	try:
		cl, addr = http_socket.accept()
	except:
		return
	try:
		print('[HTTP] Client connected from', addr)
		cl_file = cl.makefile('rwb', 0)
		print("[HTTP] Processing request 1/2...")
		request = cl_file.recv(4096).decode("utf-8")
		print("[HTTP] Processing request 2/2...")
		request = request.split("\r\n")
		print("RQ]", request)
		
		firstLine = True
		emptyCount = 0
		
		formData = {}
		notFound = False
		
		for line in request:
			if line == '':
				emptyCount += 1
				continue
			if firstLine:
				if not line.startswith("POST"):
					if not line.startswith("GET / "):
						notFound = True
					break
				firstLine = False
				continue
			if emptyCount > 0:
				formField = line.split("=")
				if len(formField) == 2:
					formData[formField[0]] = formField[1]

		result = ""
		
		if "ssid" in formData or "password" in formData:
			result += "<pre>"
		
		if "ssid" in formData:
			print("SET SSID", formData["ssid"])
			machine.nvs_setstr("system", "wifi.ssid", formData["ssid"])
			result += "WiFi SSID set to '{}'.\r\n".format(formData["ssid"])
		
		if "password" in formData:
			password = formData["password"]
			if password == "":
				password = None
			print("SET PASSWORD", password)
			
			if password:
				machine.nvs_setstr("system", "wifi.password", password)
				result += "WiFi password set to '{}'.\r\n".format(password)
			else:
				try:
					machine.nvs_erase("system", "wifi.password")
				except:
					pass
				result += "No WiFi password configured, expecting open network.\r\n"
				
		if "ssid" in formData or "password" in formData:
			result += "</pre>"

		print("[HTTP] Sending response...")

		if not notFound:

			response  = 'HTTP/1.0 200 OK\r\n\r\n'
			response += '<!doctype html><html lang="en"><head><meta charset="utf-8"><title>HSNL decennium</title></head>'
			response += '<style>html, body { padding: 0px; margin: 0px; font-family: Helvetica, Arial, Sans-Serif; background-color: #EEEEEE; }'
			response += '.header { position: absolute; left: 0px; right: 0px; top: 0px; height: 64px; background-color: #326D10; color: #FFFFFF; font-size: 25px; line-height: 64px; padding-left: 10px;}'
			response += '.content { padding: 30px; padding-top: 64px; } .right {text-align: right;}</style><body><div class="header">HSNL decennium</div><div class="content">'
			
			if (result != ""):
				response += '<h1>Response</h1>{}<br /><br /><a href="/">Return to the setup screen</a>'.format(result)
			else:
			
				response += '<form action="/" method="post" enctype="text/plain"><h1>Configuration</h1><table width="100%">'
			
				ssid = machine.nvs_getstr("system", "wifi.ssid") or ''
				password = machine.nvs_getstr("system", "wifi.password") or ''
				
				response += '<tr><td>WiFi SSID</td><td class="right"><input type="text" name="ssid" id="ssid" value="{}"/></td></tr>'.format(ssid)
				response += '<tr><td>WiFi password<br /></td><td class="right"><input type="text" name="password" value="{}"/></td></tr>'.format(password)
				
				response += '<tr><td> </td><td class="right"><input type="submit" value="Save"></td></tr></table><br /></form>'
				response += '<i>Leave the WiFi password field empty when connecting to an open network</i><br />'
				response += '<i>Connecting to enterprise networks is currently not supported!</i><br />'
				response += '<h1>List of networks</h1>'
				response += '<i>Click on a name to copy it to the WiFi SSID text field</i><br /><br />'
				
				for i in wifiList:
					ssid = i[0].decode("utf-8")
					sec  = i[5]
					response += '<a href="javascript:setSsid(\'{}\');">{} [{}]</a><br />'.format(ssid, ssid, sec)
				
				response += '<script>function setSsid(val) { document.getElementById("ssid").value = val; }</script>'
			response += '</div></body></html>'
		else:
			response = "HTTP/1.0 404 Not Found\r\n\r\n<pre>Request could not be processed.</pre>"
		cl.send(response)
		cl.close()
		print("[HTTP] Done")
	except BaseException as e:
		print(e)

wifi._STA_IF.active(True) # Needed for WiFi SSID scan
start_ap()
webserver()
dnsserver()

def buttonHandler(pressed):
	global button_pressed
	if pressed:
		system.home()

_buttons.register(0)
_buttons.attach(0, buttonHandler)

def blink(state):	
	for i in range(len(leds)):
		red = leds[i]["red"]
		grn = leds[i]["grn"]
		if state == 0:
			display.drawPixel(red[0], red[1], 0x010101)
			display.drawPixel(grn[0], grn[1], 0x000000)
		else:
			display.drawPixel(red[0], red[1], 0x000000)
			display.drawPixel(grn[0], grn[1], 0x010101)
	display.flush()

wifiList = wifi.scan()

while True:
	blink(0)
	handle_web()
	time.sleep(0.03)
	blink(1)
	handle_dns()
	time.sleep(0.03)
