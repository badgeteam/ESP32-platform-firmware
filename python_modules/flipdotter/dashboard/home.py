import ethernet, wifi, display, network, machine, time

def msg(text1, text2=""):
	display.drawFill(0x000000)
	display.drawText(0,0,text1,0xFFFFFF,"7x5")
	display.drawText(0,8,text2,0xFFFFFF,"7x5")
	display.flush()

def telnet():
	user = machine.nvs_getstr("system", "telnet.user") or "micro"
	password = machine.nvs_getstr("system", "telnet.password") or "python"
	timeout = machine.nvs_getint("system", "telnet.timeout") or 300
	network.telnet.start(user=user, password=password, timeout=timeout)


display.backlight(0xFF)
msg("Welcome")
time.sleep(5)
if ethernet.connected():
	msg("Ethernet")
	dot = "."
	while ethernet.connected() and not ethernet.hasip():
		dot = " "+dot
		if (len(dot)>10):
			dot = "."
		msg("Ethernet",dot)
		time.sleep(0.1)
	msg("Ethernet",ethernet.ifconfig()[0])
else:
	msg("WiFi")
	wifi.connect()
	dot = "."
	while not wifi.status():
		dot = " "+dot
		if (len(dot)>10):
			dot = "."
		msg("WiFi",dot)
		time.sleep(0.1)
	msg("WiFi",wifi.ifconfig()[0])

telnet()

# Small drawing application

bufferSize = display.width()*display.height()//8+1

import socket
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(("0.0.0.0", 42))

font = "7x5"
bgColor = 0x000000
fgColor = 0xFFFFFF

while(True):
	bytesAddressPair = s.recvfrom(bufferSize)
	message = bytesAddressPair[0]
	address = bytesAddressPair[1]
	#print(address, message)
	try:
		if (message.startswith(b"T")):
			text = message.decode("utf-8")[1:]
			print("Text", text)
			display.drawFill(bgColor)
			y = 0
			if font.lower() == "ocra16":
				y = -1
			if font.lower() == "roboto_regular18":
				y = -3
			if font.lower() == "roboto_regular12":
				y = -3
			if font.lower() == "dejavusans20":
				y = -4
			display.drawText(0,y,text,fgColor,font)
			display.flush()
		elif (message.startswith(b"I")):
			print("Invert")
			bgColor = 0xFFFFFF
			fgColor = 0x000000
		elif (message.startswith(b"N")):
			print("Normal")
			bgColor = 0x000000
			fgColor = 0xFFFFFF
		elif (message.startswith(b"B")):
			value = int(message[1:],16)
			print("Backlight", value)
			display.backlight(value)
		elif (message.startswith(b"F")):
			font = message.decode("utf-8")[1:]
			if font == "1":
				font = "7x5"
			if font == "2":
				font = "ocra16"
			if font == "3":
				font = "roboto_regular18"
			if font == "4":
				font = "roboto_regular12"
			if font == "5":
				font = "dejavusans20"
			print("Font", font)
		elif (message.startswith(b"C")):
			print("Clear")
			display.drawFill(bgColor)
			display.flush()
		elif (message.startswith(b"A")):
			print("Fill")
			display.drawFill(fgColor)
			display.flush()
		elif (message.startswith(b"P")):
			x = 0
			y = 0
			for i in range(len(message)-1):
				for j in range(8):
					if message[1+i]&(1<<(7-j)):
						display.drawPixel(x,y,fgColor)
					else:
						display.drawPixel(x,y,bgColor)
					x += 1
					if x >= display.width():
						x = 0
						y += 1
			display.flush()
		else:
			print("unknown", message)
		
	except BaseException as e:
		print("Error", e)
