# Simple home app for the TTGO T-Display

import machine, display, buttons, system, time, _thread, mascot, math

def onButtonA(pressed):
    if (pressed):
        system.launcher()


demoThreadRunning = False
def onButtonB(pressed):
    global demoThreadRunning
    if (pressed):
        demoThreadRunning = True
        _ = _thread.start_new_thread("DEMO", demoThread, ())
    else:
        demoThreadRunning = False

def drawNickname():
    owner = machine.nvs_getstr("owner", "name") or "BADGE.TEAM"
    display.drawFill(0xFFFF00)
    x = (display.width()-display.getTextWidth(owner, "ocra22"))//2
    y = (display.height()-display.getTextHeight(owner, "ocra22"))//2
    display.drawText(x,y,owner,0x000000,"ocra22")
    display.flush() # Send the new buffer to the display
    display.backlight(0xFF) # Turn on backlight

buttons.attach(buttons.BTN_A, onButtonA)
buttons.attach(buttons.BTN_B, onButtonB)

drawNickname()

# Demo :-)

def demoThread():
    global demoThreadRunning
    counter = 0
    fpsTrig = False
    
    w, h, _, _ = display.pngInfo(mascot.snek)
    
    display.drawFill(0xFFFF00)
    
    prev = time.ticks_ms()
    
    av = [0]*100
    
    while demoThreadRunning:
        if counter > 100:
            fpsTrig = True
        current = time.ticks_ms()
        if (current-prev) > 0:
            fps = 1000//(current-prev)
        else:
            fps = 0
        _ = av.pop(0)
        av.append(fps)
        fps = 0
        for i in range(len(av)):
            fps += av[i]
        fps = fps // len(av)
        display.drawRect(0,0,display.width()-1,20, True, 0xFFFF00)
        if fpsTrig:
            display.drawText(0,0,"{} fps".format(fps),0x000000,"ocra16")
        else:
            display.drawText(0,0,"Demo time!",0x000000,"ocra16")
        
        my = (display.height()-h)//2 - (round(math.sin(counter))-1)*10
        mx = (display.width()-w)//2 - (round(math.cos(counter))-1)*10
        display.drawPng(mx,my,mascot.snek)
        
        #Draw squares
        display.drawRect(display.width()-39-(counter % display.width()),display.height()//2-20,40,40, True, 0x00FF00)
        display.drawRect(counter % display.width(),display.height()//2-10,20,20, True, 0xFF0000)
        #Flush to display
        display.flush()
        #Clear away the squares
        display.drawRect(display.width()-39-(counter % display.width()),display.height()//2-20,40,40, True, 0xFFFF00)
        display.drawRect(counter % display.width(),display.height()//2-10,20,20, True, 0xFFFF00)
        #Clear away the mascot
        display.drawRect(mx,my,w,h,True,0xFFFF00)
        #Increment counter
        counter+=1
        #time.sleep(0.01)
        prev = current
    drawNickname()

# Terminal menu

import term, term_menu

cfg_term_menu = machine.nvs_get_u8('splash', 'term_menu') # Show a menu on the serial port instead of a prompt
if cfg_term_menu == None:
	cfg_term_menu = True # If not set the menu is shown

if cfg_term_menu:
	umenu = term_menu.UartMenu(None, None, False)
	umenu.main()
	pass
else:
	print("Welcome!")
	print("The homescreen and it's services are currently being run.")
	print("Press CTRL+C to reboot directly to a Python prompt.")
	wait = True
	while wait:
		c = machine.stdin_get(1,1)
		if c == "\x03" or c == "\x04": # CTRL+C or CTRL+D
			wait = False
	stopThreads = True
	system.shell()
