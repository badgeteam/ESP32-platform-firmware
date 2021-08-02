import machine, display, time, system, consts, easydraw, network

PREV_TGT = 2
CURR_TGT = 3

# 1) Introduction
currentState = machine.nvs_getint('system', 'factory_checked') or 0

if currentState >= PREV_TGT:
    machine.nvs_setint('system', 'factory_checked', CURR_TGT)
    system.home()

easydraw.messageCentered("FACTORY\n"+consts.INFO_FIRMWARE_NAME+"\nBuild "+str(consts.INFO_FIRMWARE_BUILD), True)
display.flush()
time.sleep(2)

# 2) Make sure that WiFi works
easydraw.messageCentered("WiFi\nScanning...", True)
display.flush()
sta_if = network.WLAN(network.STA_IF)
sta_if.active(True)
data = sta_if.scan()
if len(data) > 0:
	easydraw.messageCentered("WiFi\nFound {}".format(len(data)), True)
	display.flush()
	for item in data:
		print("SSID: {}, BSSID: {}. CHANNEL: {}, RSSI: {}, AUTHMODE: {} / {}, HIDDEN: {}".format(item[0], item[1], item[2], item[3], item[4], item[5], item[6]))
else:
	easydraw.messageCentered("WiFi\nNo network!", True)
	while True:
		time.sleep(1) #Sleep forever

# 3) Install icons (workaround)
easydraw.messageCentered("Installing...", False)
display.flush()
import dashboard.resources.png_icons

# 4) Set flag
machine.nvs_setint('system', 'factory_checked', CURR_TGT)

# 5) Show message
easydraw.messageCentered("PASSED", True)
display.flush()
