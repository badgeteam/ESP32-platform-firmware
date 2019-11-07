import wifi, time, consts

def download_info(show=False):
	import urequests as req
	result = False
	try:
		data = req.get(consts.OTA_WEB_PROTOCOL+"://"+consts.OTA_WEB_SERVER+":"+consts.OTA_WEB_PORT+consts.OTA_WEB_VERSION_PATH)
	except BaseException as e:
		print("Exception", e)
		return False
	try:
		result = data.json()
	except:
		data.close()
		return False
	data.close()
	return result

def available(update=False, show=False):
	if update:
		if not wifi.status():
			return machine.nvs_get_u8('system','OTA.ready') == 1
		info = download_info(show)
		if info:
			if info["build"] > consts.INFO_FIRMWARE_BUILD:
				machine.nvs_set_u8('system', 'OTA.ready', 1)
				return True

		machine.nvs_set_u8('system', 'OTA.ready', 0)
	return machine.nvs_get_u8('system', 'OTA.ready') == 1
