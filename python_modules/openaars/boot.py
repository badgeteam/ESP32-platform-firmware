import machine, sys, system, time, os

rtc = machine.RTC()
rtc.write(0,0)
rtc.write(1,0)

sdPower = machine.Pin(32, machine.Pin.OUT)
sdPower.value(False)

print("""

   /     \             \            /    \       
  |       |             \          |      |      
  |       `.             |         |       :     
  `        |             |        \|       |     
   \       | /       /  \\\   --__ \\       :    
    \      \/   _--~~          ~--__| \     |      
     \      \_-~                    ~-_\    |    
      \_     \        _.--------.______\|   |    
        \     \______// _ ___ _ (_(__>  \   |    
         \   .  C ___)  ______ (_(____>  |  /    
         /\ |   C ____)/      \ (_____>  |_/     
        / /\|   C_____)       |  (___>   /  \    
       |   (   _C_____)\______/  // _/ /     \   
       |    \  |__   \\_________// (__/       |  
      | \    \____)   `----   --'             |  
      |  \_          ___\       /_          _/ | 
     |              /    |     |  \            | 
     |             |    /       \  \           | 
     |          / /    |         |  \           |
     |         / /      \__/\___/    |          |
    |           /        |    |       |         |
    |          |         |    |       |         |
   ___                      _        _    ____  ____  
  / _ \ _ __   ___ _ __    / \      / \  |  _ \/ ___| 
 | | | | '_ \ / _ \ '_ \  / _ \    / _ \ | |_) \___ \ 
 | |_| | |_) |  __/ | | |/ ___ \  / ___ \|  _ < ___) |
  \___/| .__/ \___|_| |_/_/   \_\/_/   \_\_| \_\____/ 
       |_|                                            

""")

time.sleep(0.05)
os.mountsd()

#Application starting
app = rtc.read_string()
if not app:
	if not machine.nvs_getint("system", 'factory_checked') >= 2:
		app = "factory_checks"
	else:
		app = machine.nvs_getstr("system", 'default_app')
		if not app:
			app = 'openaars'

if app and not app == "shell":
	try:
		print("Starting app '%s'..." % app)
		system.__current_app__ = app
		if app:
			__import__(app)
	except BaseException as e:
		sys.print_exception(e)
		if not machine.nvs_get_u8("system", "ignore_crash"):
			print("Fatal exception in the running app!")
			system.crashedWarning()
			time.sleep(3)
			system.launcher()

if app and app == "shell":
	print("\nWelcome to the python shell of your badge!\nCheck out https://docs.badge.team/ for instructions.")
