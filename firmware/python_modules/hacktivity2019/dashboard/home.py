# Homescreen application
import machine, gc, time, uos, json, sys, system, virtualtimers, wifi, term, term_menu, orientation, display, buttons, samd, consts
import tasks.powermanagement as pm, tasks.otacheck as otacheck
import easydraw, rtc

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

# Button callbacks
def cbStartLauncher(pressed):
	if pressed:
		system.launcher(True)

def cbFeedPowerManagement(pressed):
	pm.feed()
	#pass

buttons.attach(buttons.BTN_OK,     cbStartLauncher)
buttons.attach(buttons.BTN_BACK,   cbFeedPowerManagement)
buttons.attach(buttons.BTN_DOWN,   cbFeedPowerManagement)
buttons.attach(buttons.BTN_RIGHT,  cbFeedPowerManagement)
buttons.attach(buttons.BTN_UP,     cbFeedPowerManagement)
buttons.attach(buttons.BTN_LEFT,   cbFeedPowerManagement)

# Scheduler
virtualtimers.activate(25)

# Power management
def cbSleep(idleTime=None):
	if idleTime == None:
		idleTime = virtualtimers.idle_time()
	gui_redraw = True
	drawTask(True)
	samd.backlight(0)
	system.sleep(idleTime, True)

pm.callback(cbSleep)
pm.enable()
pm.feed()

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
		display.drawText(5, position, "("+gui_app_names[gui_app_current]+")", 0x000000, "Roboto_Regular18")

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
		display.drawCircle(x, display.height()-8, size, 0, 359, i==position, 0x000000)
		x+= size + offset

def drawTask(onSleep=False):
	global gui_redraw, cfg_nickname, gui_apps, gui_app_current, ota_available
	if gui_redraw or onSleep:
		gui_redraw = False
		display.drawFill(0xFFFFFF)
		currHeight = 0
		noLine = False
		if gui_app_current < 0:
			nick = machine.nvs_getstr("owner", "name")
			if cfg_logo and cfg_nickname:
				display.drawText((display.width()-display.getTextWidth(nick, "roboto_regular12"))//2, currHeight, nick, 0x000000, "roboto_regular12")
				currHeight += display.getTextHeight(nick, "roboto_regular12")#easydraw.nickname()
				currHeight += 4
				display.drawLine(0, currHeight, display.width()-1, currHeight, 0x000000)
				currHeight += 4
			app_height = display.height()-16-currHeight
			logoHeight = drawLogo(currHeight, app_height, True)
			if logoHeight > 0:
				noLine = True
			if (not cfg_logo) and cfg_nickname and nick:
				noLine = True
				display.drawText((display.width()-display.getTextWidth(nick, "roboto_regular12"))//2, currHeight, nick, 0x000000, "roboto_regular12")
				currHeight += display.getTextHeight(nick, "roboto_regular12")
			else:
				title = "BADGE.TEAM"
				subtitle = consts.INFO_FIRMWARE_NAME 
				logoHeight = display.getTextHeight(title, "fairlight12")+display.getTextHeight(subtitle, "roboto_regular12")
				display.drawText((display.width()-display.getTextWidth(title, "fairlight12"))//2, currHeight + (app_height - logoHeight)//2,title, 0x000000, "fairlight12")
				currHeight += display.getTextHeight(title, "fairlight12")
				display.drawText((display.width()-display.getTextWidth(subtitle, "roboto_regular12"))//2, currHeight + (app_height - logoHeight)//2,subtitle, 0x000000, "roboto_regular12")
				currHeight += display.getTextHeight(subtitle, "roboto_regular12")
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
			info = 'Press OK'
		if not noLine:
			display.drawLine(0, display.height()-16, display.width(), display.height()-16, 0x000000)
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

# LED animation
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
