# File: easywifi.py
# Version: 1
# Description: Wrapper that makes using wifi simple
# License: MIT
# Authors: Renze Nicolai <renze@rnplus.nl>

import time, network, machine

state = False
failed = False

def status():
	global state
	return state

def failure():
	global failed
	return failed

def force_enable():
	global state
	state = False
	global failed
	failed = False
	enable()

def enable(showStatus=True):
	global failed
	global state
	if not state:
		nw = network.WLAN(network.STA_IF)
		if not nw.isconnected():
			nw.active(True)
			ssid = machine.nvs_getstr('badge', 'wifi.ssid')
			password = machine.nvs_getstr('badge', 'wifi.password')
			if showStatus:
				print("Connecting to '"+ssid+"'...")
			result = "?"
			if password:
				nw.connect(ssid, password)
			else:
				nw.connect(ssid)
			timeout = machine.nvs_getint('badge', 'wifi.timeout') or 30
			show = round(timeout/5)
			while not nw.isconnected():
				newShow = round(timeout/5)
				if show != newShow:
					print("("+str(round(timeout/2))+" seconds)")
					show = newShow
				time.sleep(0.5)
				timeout = timeout - 1
				if (timeout<1):
					if showStatus:
						print("Error: could not connect!")
					disable()
					failed = True
					return False
			state = True
			failed = False
			if showStatus:
				print("Connected!")
	return True

def disable():
	global state
	state = False
	global failed
	failed = False
	nw = network.WLAN(network.STA_IF)
	nw.active(False)
