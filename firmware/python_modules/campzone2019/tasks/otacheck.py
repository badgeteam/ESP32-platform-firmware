# File: otacheck.py
# Version: 1
# Description: OTA check module
# License: MIT
# Authors: Renze Nicolai <renze@rnplus.nl>

import machine, time, wifi, consts

def checking_for_update_message():
	# Inform the user that we're checking for an update
	pass

def error_message():
	# Inform the user that an error occured
	pass

def download_info():
	import urequests as requests
	checking_for_update_message()
	result = None
	try:
		url = 'https://' + consts.OTA_WEB_SERVER + ':' + consts.OTA_WEB_PORT.replace('"', '') + '/' + consts.OTA_WEB_VERSION_PATH
		print(url)
		data = requests.get(url)
		result = data.json()
		result['build'] = int(result['build'])
		result['name'] = str(result['name'])
		data.close()
	except:
		error_message()
		time.sleep(5)
	return result

def available(update=False):
	if update:
		if not wifi.status():
			wifi.connect()
			if not wifi.wait():
				return machine.nvs_getint('system','OTA.ready') or 0

		info = download_info()
		current_build = int(consts.INFO_FIRMWARE_BUILD)
		if info:
			if info["build"] > current_build:
				machine.nvs_setint('system','OTA.ready', 1)
				return True

		machine.nvs_setint('system','OTA.ready', 0)
	return machine.nvs_getint('system','OTA.ready') or 0
