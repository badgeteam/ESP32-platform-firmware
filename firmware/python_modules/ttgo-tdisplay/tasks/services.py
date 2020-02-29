# File: services.py
# Version: 5
# API version: 2
# Description: Background services for SHA2017 badge
# License: MIT
# Authors: Renze Nicolai <renze@rnplus.nl>
#          Thomas Roos   <?>

import uos, ujson, easyrtc, time, appglue, deepsleep, ugfx, badge, machine, sys, virtualtimers

services = [] #List containing all the service objects
drawCallbacks = [] #List containing draw functions

state = True
tasks_loop = []
tasks_draw = []

rtcRequired = False

tasks_wifi = []

def wifiCallback(state):
	global tasks_wifi
	if state and rtcRequired and time.time() < 1482192000:
		easyrtc.configure()
	for task in tasks_wifi:
		task(state)

def stop():
	global state, tasks_loop, tasks_draw
	if state:
		state = False
		for i in tasks_loop:
			virtualtimers.delete(i)
		for i in tasks_draw:
			virtualtimers.delete(i)

def start():
	global state, tasks_loop, tasks_draw
	if not state:
		state = True
		for i in tasks_loop:
			virtualtimers.new(1, i)
		for i in tasks_draw:
			virtualtimers.new(1, i, True)

def setDrawCallback(drawCb=None):
	global drawCallback
	drawCallback = drawCb

def setup(drawCb=None):
	global state
	state = False
	global services
	global drawCallbacks
		
	if drawCb:
		print("[SERVICES] Draw callback registered")
		global drawCallback
		drawCallback = drawCb #This might need a better name...
	
	# Status of wifi
	wifiFailed = False
	
	#Check if lib folder exists and get application list, else stop
	try:
		apps = uos.listdir('lib')
	except OSError:
		return False
	
	#For each app...
	
	global tasks_wifi
	
	drawCallbacksUnsorted = {}
	
	for app in apps:
		print("APP: "+app)
		try:
			#Try to open and read the json description
			with open('/lib/'+app+'/service.json') as f:
				description = f.read()
			description = ujson.loads(description)
		except:
			print("[SERVICES] No description found for "+app)
			continue #Or skip the app
				
		try:
			#Try to open the service itself
			with open('/lib/'+app+'/service.py') as f:
				f.close()
		except:
			print("[SERVICES] No script found for "+app)
			continue #Or skip the app
				
		rtcRequired = False # True if RTC should be set before starting service
		loopEnabled = False # True if loop callback is requested
		drawEnabled = False # True if draw callback is requested
		
		wifiInSetup = False # True if wifi needed in setup
		wifiInLoop = False # True if wifi needed in loop
		
		try:
			if description['apiVersion']!=2:
				print("[SERVICES] Service for "+app+" is not compatible with current firmware")
				continue #Skip the app
			wifiInSetup = description['wifi']['setup']
			wifiInLoop = description['wifi']['setup']
			rtcRequired = description['rtc']
			loopEnabled = description['loop']
			drawEnabled = description['draw']
		except:
			print("[SERVICES] Could not parse description of app "+app)
			continue #Skip the app
		
		print("[SERVICES] Found service for "+app)
		
		# Import the service.py script
		try:
			srv = __import__('lib/'+app+'/service')
		except BaseException as e:
			print("[SERVICES] Could not import service of app "+app+": ")
			sys.print_exception(e)
			continue #Skip the app
		
		if wifiInSetup or wifiInLoop:
			wifi.connect()
			wifiFailed = not wifi.wait(showStatus=True)
			if wifiFailed:
				print("[SERVICES] Service of app "+app+" requires wifi and wifi failed so the service has been #disabled.")
				continue
			if not srv.onWifi != None:
				tasks_wifi.append(srv.onWifi)
			

		#if rtcRequired and time.time() < 1482192000:
		#	if not wifiFailed:
		#		print("[SERVICES] RTC required, configuring...")
		#		easyrtc.configure()
		#	else:
		#		print("[SERVICES] RTC required but not available. Skipping service.")
		#		continue # Skip the app (because wifi failed and rtc not available)
				
		drawPriority = 9999
		
		try:
			drawPriority = srv.setup()
			if drawPriority == None:
				drawPriority = 9999
		except BaseException as e:
			print("[SERVICES] Exception in service setup "+app+":")
			sys.print_exception(e)
			continue
		
		if loopEnabled:
			try:
				tasks_loop.append(srv.loop)
				#virtualtimers.new(1, srv.loop)
			except:
				print("[SERVICES] Loop requested but not defined in service "+app)
			
		pos = drawPriority
		while (drawCallbacksUnsorted.get(pos) != None):
			pos = pos + 1
			
		print("[SERVICES] Draw order",pos)
			
		if drawEnabled:
			drawCallbacksUnsorted[pos] = srv
		
		# Add the script to the global service list
		services.append(srv)
	for i in sorted(drawCallbacksUnsorted):
		drawCallbacks.append(drawCallbacksUnsorted[i])

	handleDraw = False
	if len(drawCallbacks)>0:
		print("[SERVICES] The service subsystem now handles screen redraws")
		handleDraw = True
		tasks_draw.append(draw_task)
		#virtualtimers.new(1, draw_task, True)
	return handleDraw

def draw_task():
    global drawCallback #The function that allows us to hook into our host
    
    if drawCallback == None:
		return 100 #Don't draw if we can't hook
    
    global drawCallbacks #The functions of the services
    requestedInterval = 99999999
    y = ugfx.height()
        
    drawCallback(False) # Prepare draw
    
    deleted = []

    for i in range(0, len(drawCallbacks)):
        rqi = 0
        try:
            cb = drawCallbacks[i].draw
	    try:
		[rqi, space_used] = cb(y, False)
	    except:
		[rqi, space_used] = cb(y)
            y = y - space_used
        except BaseException as e:
            print("[SERVICES] Exception in service draw:")
            sys.print_exception(e)
            deleted.append(cb)
            continue
        if rqi>0 and rqi<requestedInterval:
            # Service wants to loop again in rqi ms
            requestedInterval = rqi
        elif rqi<=0:
            # Service doesn't want to draw again until next wakeup
            deleted.append(cb)
    
    for i in range(0,len(deleted)):
        dcb = deleted[i]
        print("[DEBUG] Deleted draw callback: ",dcb)
        drawCallbacks = list(dcb for dcb in drawCallbacks if dcb!=deleted[i])
    
    if requestedInterval<1000:
        #Draw at most once a second
        print("[SERVICES] Can't draw more than once a second!")
        requestedInterval = 1000
    
    if requestedInterval>=99999999:
        print("[SERVICES] No draw interval returned.")
        requestedInterval = -1

    retVal = 0
    
    if len(drawCallbacks)>0 and requestedInterval>=0:
        #print("[SERVICES] New draw requested in "+str(requestedInterval))
        retVal = requestedInterval
    drawCallback(True) # Complete draw
    return retVal

def force_draw(goingToSleep=False):
    global drawCallbacks
    if len(drawCallbacks)>0:
        y = ugfx.height()
        for srv in drawCallbacks:
            try:
                if not goingToSleep:
                    cb = srv.draw
                else:
                    try:
                        cb = srv.draw_going_to_sleep
                    except:
                        cb = srv.draw
		try:
		    [rqi, space_used] = cb(y, True)
		except:
		    [rqi, space_used] = cb(y)
                y = y - space_used
            except BaseException as e:
                print("[SERVICES] Exception in service draw: ")
                sys.print_exception(e)
