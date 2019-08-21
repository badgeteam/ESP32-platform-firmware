import wifi, badge, time, consts

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
			return badge.nvs_get_u8('badge','OTA.ready',0)

		info = download_info(show)
		if info:
			if info["build"] > consts.INFO_FIRMWARE_BUILD:
				badge.nvs_set_u8('badge','OTA.ready',1)
				return True

		badge.nvs_set_u8('badge','OTA.ready',0)
	return badge.nvs_get_u8('badge','OTA.ready',0)
