# Home menu application

import ugfx, time, badge, machine, gc, system, virtualtimers, easydraw, wifi, rtc, term, term_menu, orientation, tasks.powermanagement as pm, uos, json, sys, tasks.otacheck as otacheck

# Default values
default_logo = '/media/hackerhotel.png'

# Read splashscreen configuration from NVS
cfg_term_menu        = badge.nvs_get_u8('splash', 'term_menu', True)
cfg_wifi             = badge.nvs_get_u8('splash', 'wifi', True)
cfg_services         = badge.nvs_get_u8('splash', 'services', True)
cfg_logo             = badge.nvs_get_str('splash', 'logo', default_logo)
cfg_nickname         = badge.nvs_get_u8('splash', 'nickname', True)
cfg_led_animation    = badge.nvs_get_str('splash', 'ledApp', None)

# Small hack to install logo if needed
try:
	media = uos.listdir("/media")
	if not "hackerhotel.png" in media:
		raise(BaseException("Logo not available"))
except:
	import dashboard.resources.png_hackerhotel

try:
	media = uos.listdir("/media")
	icons = ["alert", "bell", "bug", "busy", "charge", "crown", "earth", "flag", "music", "ok", "wifi", "usb"]
	for icon in icons:
		if not icon+".png" in media:
			raise(BaseException(""))
except:
	import dashboard.resources.png_icons

# Initialise the default button actions
def btn_start(pressed):
	if pressed:
		system.launcher(True)

def btn_unhandled(pressed):
	pm.feed()

ugfx.input_attach(ugfx.BTN_START, btn_start)
ugfx.input_attach(ugfx.BTN_A, btn_unhandled)
ugfx.input_attach(ugfx.BTN_B, btn_unhandled)
ugfx.input_attach(ugfx.JOY_UP, btn_unhandled)
ugfx.input_attach(ugfx.JOY_DOWN, btn_unhandled)
ugfx.input_attach(ugfx.JOY_LEFT, btn_unhandled)
ugfx.input_attach(ugfx.JOY_RIGHT, btn_unhandled)

# Task scheduler
virtualtimers.activate(25)

# Power management
def onSleep(idleTime=None):
	if idleTime == None:
		idleTime = virtualtimers.idle_time()
	gui_redraw = True
	drawTask(True)
	if (idleTime >= 86400000): #One day
		idleTime = 0
	if idleTime < 1:
		term.header(True, "Sleeping until touchbutton is pressed...")
	else:
		term.header(True, "Sleeping for "+str(idleTime)+"ms...")
	system.sleep(idleTime)

pm.callback(onSleep)
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
		#print("WiFi status changed", wifi_status_curr)
		pm.feed()
		wifi_status_prev = wifi_status_curr
		gui_redraw = True
		if wifi_status_curr:
			ota_available = otacheck.available(True)
			#print("Check OTA",ota_available)
	return 1000

virtualtimers.new(0, wifiTask, True)

# Services
gui_apps = []
gui_app_names = []
gui_app_current = -1

def next_app(pressed):
	global gui_apps, gui_app_current, gui_redraw
	pm.feed()
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
	pm.feed()
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
		ugfx.string(5, position, "("+gui_app_names[gui_app_current]+")", "Roboto_Regular18", ugfx.BLACK)
	

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

	ugfx.input_attach(ugfx.JOY_LEFT, prev_app)
	ugfx.input_attach(ugfx.JOY_RIGHT, next_app)

# Basic UI elements and drawing task
orientation.default()
gui_redraw = True

def drawLogo(offset = 0, max_height = ugfx.height(), center = True):
	global cfg_logo, default_logo
	try:
		info = badge.png_info(cfg_logo)
	except:
		try:
			cfg_logo = default_logo
			info = badge.png_info(cfg_logo)
		except:
			return 0
	width = info[0]
	height = info[1]
	if width > ugfx.width():
		print("Image too large (x)")
		return
	if height > ugfx.height():
		print("Image too large (y)")
	x = int((ugfx.width() - width) / 2)
	if center:
		if max_height - height < 0:
			print("Not enough space for logo",max_height,height)
			#return 0
			y = 0
		else:
			y = int((max_height - height) / 2) + offset
	else:
		y = offset
	try:
		#badge.png(x,y,cfg_logo)
		ugfx.display_image(x,y,cfg_logo)
		return height
	except BaseException as e:
		sys.print_exception(e)
	return 0

def drawPageIndicator(amount, position):
	x = 5
	size = 4
	offset = 6
	for i in range(amount):
		if i==position:
			ugfx.fill_circle(x, ugfx.height()-8, size, ugfx.BLACK)
		else:
			ugfx.circle(x, ugfx.height()-8, size, ugfx.BLACK)
		x+= size + offset

def drawTask(onSleep=False):
	global gui_redraw, cfg_nickname, gui_apps, gui_app_current, ota_available
	if gui_redraw or onSleep:
		gui_redraw = False
		ugfx.clear(ugfx.WHITE)
		currHeight = 0
		if gui_app_current < 0:
			if cfg_nickname:
				currHeight += easydraw.nickname()
				currHeight += 4
				ugfx.line(0, currHeight, ugfx.width(), currHeight, ugfx.BLACK)
				currHeight += 4
			app_height = ugfx.height()-16-currHeight
			logoHeight = drawLogo(currHeight, app_height, True)
		else:
			display_app(currHeight)

		if onSleep:
			info = 'Sleeping...'
		#elif badge.safe_mode():
		#	info = "Recovery mode"
		#elif not rtc.isSet():
		#	info = "Clock not set"
		elif ota_available:
			info = "Update available!"
		#elif wifi_status_curr:
		#	info = "WiFi connected"
		else:
			info = 'Press START'
		ugfx.line(0, ugfx.height()-16, ugfx.width(), ugfx.height()-16, ugfx.BLACK)
		easydraw.disp_string_right_bottom(0, info)
		if len(gui_apps) > 0:
			drawPageIndicator(len(gui_apps), gui_app_current)
		ugfx.flush(ugfx.LUT_NORMAL)
	return 1000

virtualtimers.new(0, drawTask, True)

# Free up space in RAM
gc.collect()

# Set the RTC if needed and WiFi is allowed
if not rtc.isSet() and cfg_wifi:
	wifi.connect() #Connecting to WiFi automatically sets the time using NTP (see wifiTask)

# LED animation
if cfg_led_animation != None:
	badge.leds_init()
	badge.leds_enable()
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
	umenu = term_menu.UartMenu(onSleep, pm, badge.safe_mode())
	umenu.main()
