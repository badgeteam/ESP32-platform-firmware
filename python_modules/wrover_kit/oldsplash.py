import ugfx, time, badge, machine, deepsleep, gc
import system, virtualtimers
import easydraw

import tasks.powermanagement as pm
import tasks.otacheck as otac
import tasks.services as services

# Graphics

def nickname(y = 25, font = "PermanentMarker36", color = ugfx.BLACK):
    nick = badge.nvs_get_str("owner", "name", 'Henk de Vries')
    ugfx.string_box(0,y,296,38, nick, font, color, ugfx.justifyCenter)

def battery(on_usb, vBatt, charging):
    vMin = badge.nvs_get_u16('batt', 'vmin', 3500) # mV
    vMax = badge.nvs_get_u16('batt', 'vmax', 4100) # mV
    if charging and on_usb:
        try:
            badge.png(0,0,'/media/charge.png')
        except:
            ugfx.string(0, 0, "CHRG",'Roboto_Regular12',ugfx.BLACK)
    elif on_usb:
        try:
            badge.png(0,0,'/media/usb.png')
        except:
            ugfx.string(0, 0, "USB",'Roboto_Regular12',ugfx.BLACK)
    else:
        width = round((vBatt-vMin) / (vMax-vMin) * 44)
        if width < 0:
            width = 0
        elif width > 38:
            width = 38
        ugfx.box(2,2,46,18,ugfx.BLACK)
        ugfx.box(48,7,2,8,ugfx.BLACK)
        ugfx.area(3,3,width,16,ugfx.BLACK)

def draw(mode, goingToSleep=False):
    ugfx.orientation(0)
    if mode:
        # We flush the buffer and wait
        ugfx.flush(ugfx.LUT_FULL)
        badge.eink_busy_wait()
    else:
        # We prepare the screen refresh
        ugfx.clear(ugfx.WHITE)
        if goingToSleep:
            info1 = 'Sleeping...'
            info2 = 'Press any key to wake up'
        else:
            info1 = 'Press start to open the launcher'
            if otac.available(False):
                info2 = 'Press select to start OTA update'
            else:
                info2 = ''

	def disp_string_right(y, s):
	    l = ugfx.get_string_width(s,"Roboto_Regular12")
	    ugfx.string(296-l, y, s, "Roboto_Regular12",ugfx.BLACK)

	disp_string_right(0, info1)
	disp_string_right(12, info2)

	if badge.safe_mode():
	    disp_string_right(92, "Safe Mode - services disabled")
	    disp_string_right(104, "Sleep disabled - will drain battery quickly")
	    disp_string_right(116, "Press Reset button to exit")
        
        nickname()
        
        on_usb = pm.usb_attached()
        vBatt = badge.battery_volt_sense()
        vBatt += vDrop
        charging = badge.battery_charge_status()

        battery(on_usb, vBatt, charging)
        
        if vBatt>500:
            ugfx.string(52, 0, str(round(vBatt/1000, 1)) + 'v','Roboto_Regular12',ugfx.BLACK)

# About

def splash_about_countdown_reset():
    global splashAboutCountdown
    splashAboutCountdown = badge.nvs_get_u8('splash', 'about.amount', 10)

def splash_about_countdown_trigger():
    global splashAboutCountdown
    try:
        splashAboutCountdown
    except:
        splash_about_countdown_reset()

    splashAboutCountdown -= 1
    if splashAboutCountdown<0:
        system.start('magic', False)
    else:
        print("[SPLASH] Magic in "+str(splashAboutCountdown)+"...")

# Button input

def splash_input_start(pressed):
    # Pressing start always starts the launcher
    if pressed:
        system.start("launcher", False)

def splash_input_a(pressed):
    if pressed:
        splash_about_countdown_trigger()
        pm.feed()

def splash_input_select(pressed):
    if pressed:
        if otac.available(False):
            system.ota()
        pm.feed()

def splash_input_other(pressed):
    if pressed:
        pm.feed()

def splash_input_init():
    print("[SPLASH] Inputs attached")
    ugfx.input_init()
    ugfx.input_attach(ugfx.BTN_START, splash_input_start)
    ugfx.input_attach(ugfx.BTN_A, splash_input_a)
    ugfx.input_attach(ugfx.BTN_B, splash_input_other)
    ugfx.input_attach(ugfx.BTN_SELECT, splash_input_select)
    ugfx.input_attach(ugfx.JOY_UP, splash_input_other)
    ugfx.input_attach(ugfx.JOY_DOWN, splash_input_other)
    ugfx.input_attach(ugfx.JOY_LEFT, splash_input_other)
    ugfx.input_attach(ugfx.JOY_RIGHT, splash_input_other)

# Power management
 
def onSleep(idleTime):
    draw(False, True)
    services.force_draw(True)
    draw(True, True)

### PROGRAM

# Calibrate battery voltage drop
if badge.battery_charge_status() == False and pm.usb_attached() and badge.battery_volt_sense() > 2500:
    badge.nvs_set_u16('splash', 'bat.volt.drop', 5200 - badge.battery_volt_sense()) # mV
    print('Set vDrop to: ' + str(4200 - badge.battery_volt_sense()))
vDrop = badge.nvs_get_u16('splash', 'bat.volt.drop', 1000) - 1000 # mV

splash_input_init()
splash_about_countdown_reset()

if not badge.safe_mode():
    services.setup(draw) # Start services

draw(False)
services.force_draw()
draw(True)

gc.collect()
    
virtualtimers.activate(25)
pm.callback(onSleep)
pm.feed()
