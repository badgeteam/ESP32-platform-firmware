import ugfx, time, badge, machine, deepsleep, gc
import appglue, virtualtimers
import easydraw, easywifi, easyrtc

import tasks.powermanagement as pm
import tasks.otacheck as otac
import tasks.services as services
import term, term_menu

# Graphics


def ledAnimationDisplay(i):
	if i > 5:
		i = 5-i
	for j in range(6):
		if j==i:
			badge.led(j, 255, 0, 0)
		else:
			if j==5-i:
				badge.led(j, 0, 255, 0)
			else:
				badge.led(j, 0, 0, 0)
	
ledAnimationStep=0
ledAnimationCount=0

def ledAnimationTask():
    global ledAnimationStep
    global ledAnimationCount
    ledAnimationStep +=1
    if ledAnimationStep<11:
        ledAnimationCount+=1
        ledAnimationDisplay(ledAnimationStep)
        return 100
    elif ledAnimationCount<4:
        ledAnimationStep=0
        for j in range(6):
            badge.led(j, 0, 0, 0)
        return 100
    else:
        for j in range(6):
            badge.led(j, 0, 0, 0)
        ledAnimationStep=0
        return 600000

def draw(mode, goingToSleep=False):
	info1 = ''
	info2 = ''
	if mode:
		# We flush the buffer and wait
		ugfx.flush(ugfx.GREYSCALE)
	else:
		# We prepare the screen refresh
		ugfx.clear(ugfx.WHITE)
		easydraw.nickname()
		if goingToSleep:
			info = 'Sleeping...'
		elif badge.safe_mode():
			info = "(Services disabled!)"
		elif otac.available(False):
			info = 'Update available!'
		else:
			info = ''
		easydraw.disp_string_right_bottom(0, info)

# Button input

def splash_input_start(pressed):
	# Pressing start always starts the launcher
	if pressed:
		appglue.start_app("launcher", False)

def splash_input_other(pressed):
	if pressed:
		pm.feed()

def splash_input_init():
	print("[SPLASH] Inputs attached")
	ugfx.input_init()
	ugfx.input_attach(ugfx.BTN_START, splash_input_start)
	ugfx.input_attach(ugfx.BTN_B, splash_input_other)
	ugfx.input_attach(ugfx.JOY_UP, splash_input_other)
	ugfx.input_attach(ugfx.JOY_DOWN, splash_input_other)
	ugfx.input_attach(ugfx.JOY_LEFT, splash_input_other)
	ugfx.input_attach(ugfx.JOY_RIGHT, splash_input_other)

# Power management

def goToSleep():
	onSleep(virtualtimers.idle_time())

def onSleep(idleTime):
	draw(False, True)
	services.force_draw(True)
	draw(True, True)

### PROGRAM

# badge.backlight(255)
#
# splash_input_init()

# post ota script
import post_ota

#setupState = badge.nvs_get_u8('badge', 'setup.state', 0)
#if setupState < 3: #First boot (3 for SHA compat)
#	print("[SPLASH] Force ota check...")
#	badge.nvs_set_u8('badge', 'setup.state', 3)
#	if not easywifi.failure():
#		otac.available(True)
#else: # Normal boot
#	print("[SPLASH] Normal boot... ("+str(machine.reset_cause())+")")
#	#if (machine.reset_cause() != machine.DEEPSLEEP_RESET):
#	#	print("... from reset: checking for ota update")
#	#	if not easywifi.failure():
#	#		otac.available(True)

# RTC ===
#if time.time() < 1482192000:
#	easyrtc.configure()
# =======

print('[BOOT] Choosing boot mode')
if badge.safe_mode():
	print('[BOOT] Safe mode')
	# draw(False)
	# services.force_draw()
	# draw(True)
else:
	print('[BOOT] Normal boot mode')
	print('[BOOT] Finding services')
	have_services = services.setup() # Start services
	if not have_services:
		print('[BOOT] No services found')
		# draw(False)
		# services.force_draw()
		# draw(True)

print('[BOOT] Disabling WiFi and running garbage collection')
easywifi.disable()
gc.collect()

# virtualtimers.activate(25)
# pm.callback(onSleep)
# pm.feed()
#
# virtualtimers.new(10, ledAnimationTask)

print('[BOOT] Launching UART menu')
umenu = term_menu.UartMenu(goToSleep, pm, badge.safe_mode())
umenu.main()
