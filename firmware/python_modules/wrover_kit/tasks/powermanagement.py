import virtualtimers, deepsleep, badge, sys

requestedStandbyTime = 0
onSleepCallback = None

timeout = badge.nvs_get_u16('badge', 'sleep', 10000) #Default to 10 seconds

disabled = False
enabled = False

def disable():
	global disabled, enabled
	disabled = True
	enabled = False
	kill()
	
def enable():
	global disabled, enabled, timeout
	if timeout < 1: #Disabled by setting
		return
	disabled = False
	enabled = True
	feed()

def resume():
	global disabled, enabled
	if disabled and enabled:
		enable()

def state():
	if disabled:
		return -1
	return timeout

def usb_attached():
	return badge.usb_volt_sense() > 4500

def pm_task():
	''' The power management task [internal function] '''
	global requestedStandbyTime
	global timeout
	global disabled
	
	if disabled:
		return timeout
	
	idleTime = virtualtimers.idle_time()
	
	if idleTime > 30000 and not badge.safe_mode() and not ( usb_attached() and badge.nvs_get_u8('badge', 'usb_stay_awake', 0) != 0 ):
		global onSleepCallback
		if not onSleepCallback==None:
			print("[Power management] Running onSleepCallback...")
			try:
				onSleepCallback(idleTime)
			except BaseException as e:
				print("[ERROR] An error occured in the on sleep callback.")
				sys.print_exception(e)
		deepsleep.start_sleeping(idleTime)
	
	return timeout

def feed():
    ''' Start / resets the power management task '''
    global timeout, enabled
    if timeout < 1: #Disabled by setting
		return
    if not virtualtimers.update(timeout, pm_task):
		enabled = True
		virtualtimers.new(timeout, pm_task, True)

def kill():
    ''' Kills the power management task '''
    virtualtimers.delete(pm_task)
    global disabled, enabled
    disabled = True
    enabled = False
    
def callback(cb):
    ''' Set a callback which is run before sleeping '''
    global onSleepCallback
    onSleepCallback = cb
    
def set_timeout(t):
    ''' Set timeout '''
    global timeout
    timeout = t
