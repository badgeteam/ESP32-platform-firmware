# Homescreen application
import machine, gc, time, uos, json, sys, system, virtualtimers, wifi, term, term_menu, orientation, display, buttons, neopixel
import tasks.powermanagement as pm, tasks.otacheck as otacheck
import easydraw, rtc

COLOR_BG = 0x000000
COLOR_FG = 0xFFFFFF

LOGO = b"\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR\x00\x00\x00\x80\x00\x00\x00\x1a\x01\x00\x00\x00\x00W\xb8iC\x00\x00\x00\xecIDAT\x18\xd3\x95\xd1\xa1N\x041\x10\x06\xe0\x7f\xd2f\xe7\xc4\xe6\xba\t\xa6\x82d\t\xb9\x07\xa8\xac \xdc\x9cC\xf0\x10K\x82D\x9c<\x81\xd8K\xf0X\x1e\xa7\x0e\x1e\xa3\x12Y\xb9\x82P\xba{\x9b\xbd\xe0`\xe4'\xfe\xcc\xfc\x03\xfcs\x08P\xdf\x80IxF\x1b\x11g\xf0&\xd2{\xbfM*\x13\x07(\x86w\xa8\x10\x1e\x06\x93\x8f\x1c\x0b\x84\x02\xb5\xef\xe4\xc0Q\xef\xfa\x11\x0e\x0e67\xe2)\xe8]\xc9@*pAo\xe2U(\x993\\\xe3C\xec\x02{\x87\r\xeeF\x80>\xc3\xa3\\\xaa\xdc\xcf`\xe2S\x81[\xce/\x0b\xdcc#-\x0f\xfa\x04\x0e\xab\x0eZ\x0cc\x06\x0f{\x85Zx\x017\xc2Jx\r\xfa*@#\xb8\xd73\x94[`\x9b\xbe)`\xa7=&\xb0pR\x99<\xc1\x1a\x05jtR\xb5\xf9\x98\xa6\x82n\xf8\x93)H\xdd\x0et\x02g\x12e\x88k\x7f\xf7\xbb\r\x7f|\xc4\x0f\xc0\xa9K\xfd1u\xb7\xde\x00\x00\x00\x00IEND\xaeB`\x82"

# Read configuration from NVS or apply default values
cfg_term_menu = machine.nvs_get_u8('splash', 'term_menu') # Show a menu on the serial port instead of a prompt
if cfg_term_menu == None:
	cfg_term_menu = True # If not set the menu is shown

cfg_wifi = machine.nvs_get_u8('splash', 'wifi') # Allow the use of WiFi on the splash screen
if cfg_wifi == None:
	cfg_wifi = False # If not set the use of WiFi is not allowed

cfg_services = machine.nvs_get_u8('splash', 'services') # Enable splash screen services (fun but dangerous)
if cfg_services == None:
	cfg_services = False # If not set services are disabled

cfg_logo = machine.nvs_getstr('splash', 'logo') # Filename of a PNG image to show on the splash screen

cfg_nickname = machine.nvs_get_u8('splash', 'nickname') # Show the nickname of the user on the splash screen
if cfg_nickname == None:
	cfg_nickname = True # If not set we want to show the nickname

cfg_greyscale = machine.nvs_get_u8('splash', 'greyscale') # Use greyscale mode
if cfg_greyscale == None:
	cfg_greyscale = False # Disabled by default

cfg_led_animation = machine.nvs_getstr('splash', 'ledApp') # Application which shows a LED animation while the splash screen is visible

cfg_nick_text = machine.nvs_getstr("owner", "name")
if not cfg_nick_text:
	cfg_nick_text = "Welcome to Disobey 2020!"

# Button callbacks
def cbStartLauncher(pressed):
	if pressed:
		system.launcher(True)

def cbFeedPowerManagement(pressed):
	pass
	#pm.feed()

# Flashlight
flashlightStatus = False
def cbFlashlight(pressed):
	global led_app, flashlightStatus
	if pressed:
		if flashlightStatus:
			#pm.enable()
			neopixel.send(bytes([0x00]*3*12))
		else:
			#pm.disable()
			neopixel.send(bytes([0xFF]*3*12))

		if led_app:
			try:
				if flashlightStatus:
					led_app.resume()
				else:
					led_app.pause()
			except:
				pass
		flashlightStatus = not flashlightStatus

buttons.attach(buttons.BTN_A,      cbFeedPowerManagement)
buttons.attach(buttons.BTN_B,      cbFeedPowerManagement)
buttons.attach(buttons.BTN_START,  cbStartLauncher      )
buttons.attach(buttons.BTN_SELECT, cbFeedPowerManagement)
buttons.attach(buttons.BTN_DOWN,   cbFeedPowerManagement)
buttons.attach(buttons.BTN_RIGHT,  cbFeedPowerManagement)
buttons.attach(buttons.BTN_UP,     cbFeedPowerManagement)
buttons.attach(buttons.BTN_LEFT,   cbFeedPowerManagement)
buttons.attach(buttons.BTN_FLASH,  cbFlashlight         )

# Scheduler
virtualtimers.activate(25)

# Power management
def cbSleep(idleTime=None):
	global ledTask, scrollerTask
	neopixel.send(bytes([0x00]*3*12))
	if idleTime == None:
		idleTime = virtualtimers.idle_time()
	gui_redraw = True
	virtualtimers.delete(scrollerTask)
	virtualtimers.delete(ledTask)
	display.windowHide("scroller")
	drawTask(True)
	display.flush()
	time.sleep(0.1)
	system.sleep(idleTime, True)

#pm.callback(cbSleep)
#pm.enable()
#pm.feed()

# WiFi
wifi_status_prev = False
wifi_status_curr = False
ota_available    = False

def wifiTask():
	global wifi_status_prev, wifi_status_curr, gui_redraw, ota_available
	wifi_status_prev = wifi_status_curr
	wifi_status_curr = wifi.status()
	if wifi_status_curr:
		wifi.ntp(True)
	if wifi_status_curr != wifi_status_prev:
		#pm.feed()
		wifi_status_prev = wifi_status_curr
		gui_redraw = True
		if wifi_status_curr:
			ota_available = otacheck.available(True)
	return 1000

virtualtimers.new(50, wifiTask, True)

# Services
gui_apps = []
gui_app_names = []
gui_app_current = -1

def next_app(pressed):
	global gui_apps, gui_app_current, gui_redraw
	#pm.feed()
	if pressed:
		if gui_app_current < len(gui_apps) - 1:
			if gui_app_current >= 0:
				try:
					gui_apps[gui_app_current].focus(False)
				except BaseException as e:
					sys.print_exception(e)
			gui_app_current += 1
			try:
				gui_apps[gui_app_current].focus(True)
			except BaseException as e:
				sys.print_exception(e)
			gui_redraw = True

def prev_app(pressed):
	global gui_apps, gui_app_current, gui_redraw
	#pm.feed()
	if pressed:
		if gui_app_current >= 0:
			try:
				gui_apps[gui_app_current].focus(False)
			except BaseException as e:
				sys.print_exception(e)
			gui_app_current -= 1
			if gui_app_current >= 0:
				try:
					gui_apps[gui_app_current].focus(True)
				except BaseException as e:
					sys.print_exception(e)
			gui_redraw = True

def display_app(position):
	try:
		gui_apps[gui_app_current].draw(position)
	except BaseException as e:
		sys.print_exception(e)
		display.drawText(5, position, "("+gui_app_names[gui_app_current]+")", COLOR_FG, "Roboto_Regular18")

def redraw_cb():
	global gui_redraw
	gui_redraw = True

if cfg_services:
	try:
		f = open('/services.json', 'r')
		cfg = f.read()
		f.close()
		cfg = json.loads(cfg)
	except:
		cfg = '{"apps": []}'
		try:
			f = open('/services.json', 'w')
			f.write(cfg)
			f.close()
		except:
			pass
		cfg = json.loads(cfg)
	
	try:
		for app in cfg['apps']:
			try:
				new_app = __import__("/lib/"+app+"/srv")
				try:
					new_app.init(redraw_cb)
				except:
					new_app.init()
				gui_apps.append(new_app)
				gui_app_names.append(app)
			except BaseException as e:
				sys.print_exception(e)
	except:
		print("Error while loading services")
		pass
	
	gui_app_current = -1

	buttons.attach(buttons.BTN_LEFT, prev_app)
	buttons.attach(buttons.BTN_RIGHT, next_app)

# Basic UI elements and drawing task
orientation.default()
gui_redraw = True

def drawLogo(offset = 0, max_height = display.height(), center = True):
	global cfg_logo
	try:
		info = display.pngInfo(cfg_logo)
	except:
		return 0
	width = info[0]
	height = info[1]
	if width > display.width():
		print("Image too large (x)")
		return
	if height > display.height():
		print("Image too large (y)")
	x = int((display.width() - width) / 2)
	if center:
		if max_height - height < 0:
			#print("Not enough space for logo",max_height,height)
			#return 0
			y = 0
		else:
			y = int((max_height - height) / 2) + offset
	else:
		y = offset
	try:
		display.drawPng(x,y,cfg_logo)
		return height
	except BaseException as e:
		sys.print_exception(e)
	return 0

def drawPageIndicator(amount, position):
	x = 5
	size = 4
	offset = 6
	for i in range(amount):
		display.drawCircle(x, display.height()-8, size, 0, 359, i==position, COLOR_FG)
		x+= size + offset

def drawTask(onSleep=False):
	global gui_redraw, cfg_nickname, gui_apps, gui_app_current, ota_available
	if gui_redraw or onSleep:
		gui_redraw = False
		display.drawFill(COLOR_BG)
		currHeight = 0
		noLine = False
		if gui_app_current < 0:
			if cfg_logo:
				if cfg_nickname:
					# Logo and nickename enabled, first print nickname
					display.drawText((display.width()-display.getTextWidth(cfg_nick_text, "roboto_regular12"))//2, currHeight, cfg_nick_text, COLOR_FG, "roboto_regular12")
					currHeight += display.getTextHeight(cfg_nick_text, "roboto_regular12")#easydraw.nickname()
					currHeight += 4
					display.drawLine(0, currHeight, display.width()-1, currHeight, COLOR_FG)
					currHeight += 4
				#Print logo
				app_height = display.height()-16-currHeight
				logoHeight = drawLogo(currHeight, app_height, True)
				if logoHeight > 0:
					noLine = True
				if logoHeight < 1:
					#Logo enabled but failed to display
					title = "BADGE.TEAM"
					subtitle = "PLATFORM"
					logoHeight = display.getTextHeight(title, "permanentmarker22")+display.getTextHeight(subtitle, "fairlight12")
					display.drawText((display.width()-display.getTextWidth(title, "permanentmarker22"))//2, currHeight + (app_height - logoHeight)//2,title, COLOR_FG, "permanentmarker22")
					currHeight += display.getTextHeight(title, "permanentmarker22")
					display.drawText((display.width()-display.getTextWidth(subtitle, "fairlight12"))//2, currHeight + (app_height - logoHeight)//2,subtitle, COLOR_FG, "fairlight12")
					currHeight += display.getTextHeight(subtitle, "fairlight12")
			else:
				noLine = True
				display.drawPng(0,0,LOGO)
		else:
			display_app(currHeight)

		if onSleep:
			info = 'Sleeping...'
		#elif not rtc.isSet():
		#	info = "RTC not available"
		elif ota_available:
			info = "Update available!"
		#elif wifi_status_curr:
		#	info = "WiFi connected"
		else:
			info = ""#'Press START'
		if not noLine:
			display.drawLine(0, display.height()-16, display.width(), display.height()-16, COLOR_FG)
		easydraw.disp_string_right_bottom(0, info)
		if len(gui_apps) > 0:
			drawPageIndicator(len(gui_apps), gui_app_current)
		if cfg_greyscale:
			display.flush(display.FLAG_LUT_GREYSCALE)
		else:
			display.flush(display.FLAG_LUT_NORMAL)
	return 1000

virtualtimers.new(0, drawTask, True)

# Free up space in RAM
gc.collect()

# Set the RTC if needed and WiFi is allowed
if not rtc.isSet() and cfg_wifi:
	wifi.connect() #Connecting to WiFi automatically sets the time using NTP (see wifiTask)

# Test code for the scroller
#import virtualtimers, display
#virtualtimers.activate(25)
#COLOR_FG = 0xFFFFFF
#COLOR_BG = 0x000000
#cfg_logo = False
#cfg_nickname = True
#cfg_nick_text = "Hello world"

# Text scroller
display.windowCreate("scroller", 512, 32) #Workaround!!! windows get corrupted when size is not in units of 8
display.windowShow("scroller")
display.windowMove("scroller", 65, display.height()-22) # Move out of visible screen
display.drawFill("scroller", COLOR_BG)
display.drawText("scroller", 0, 0, cfg_nick_text, COLOR_FG, "permanentmarker22")

scrollerStartPos = 129
scrollerEndPos = -display.getTextWidth(cfg_nick_text, "permanentmarker22") - 128
scrollerPos = scrollerStartPos

offsetTest = 0

def scrollerTask():
	global scrollerPos, scrollerEndPos, scrollerStartPos, offsetTest
	display.windowMove("scroller", scrollerPos, display.height()-22-offsetTest) 
	scrollerPos-=3
	if scrollerPos < scrollerEndPos:
		scrollerPos = scrollerStartPos
	display.flush()
	return 25

if not cfg_logo and cfg_nickname:
	virtualtimers.new(25, scrollerTask, True)

# LED animation

ledValue = 4
ledDirection = False
def ledTask():
	global ledValue, ledDirection
	if not flashlightStatus:
		neopixel.send(bytes([0, 4+ledValue, 0]*5 + [0, 4+24-ledValue, 0]*7))
	if ledDirection:
		ledValue -=2
		if ledValue <= 0:
			levValue = 0
			ledDirection = False
	else:
		ledValue += 6
		if ledValue >= 24:
			levValue = 24
			ledDirection = True
	return 25

led_app = None
if cfg_led_animation != None:
	try:
		led_app = __import__('/lib/'+cfg_led_animation+'/ledsrv')
		try:
			led_app.init()
		except:
			pass
		virtualtimers.new(0, led_app.step)
	except:
		pass
else:
	virtualtimers.new(25, ledTask, True)

# Terminal menu

if cfg_term_menu:
	umenu = term_menu.UartMenu(cbSleep, pm, False)
	umenu.main()
else:
	print("Welcome!")
	print("The homescreen and it's services are currently being run.")
	print("Press CTRL+C to reboot directly to a Python prompt.")
	wait = True
	while wait:
		c = machine.stdin_get(1,1)
		if c == "\x03" or c == "\x04": # CTRL+C or CTRL+D
			wait = False
	import shell
