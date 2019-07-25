import esp, machine, sys, system, os, consts
import rgb

# Clear OTA boot magic
esp.rtcmem_write(0,0)
esp.rtcmem_write(1,0)

# Set LED brightness
brightness = machine.nvs_getint('system', 'brightness')
if not brightness:
    brightness = (rgb.MAX_BRIGHTNESS - 2)
rgb.setbrightness(brightness)

#Application starting
app = esp.rtcmem_read_string()
if app:
    esp.rtcmem_write_string("")
else:
    if not machine.nvs_getint("system", 'factory_checked'):
        app = "factory_checks"
    else:
        app = machine.nvs_getstr("system", 'default_app')
        if not app:
            app = 'dashboard.home'

if app and not app == "shell":
    try:
        print("Starting app '%s'..." % app)
        system.__current_app__ = app
        if app:
            import buttons  # Initialise buttons so by default apps exit on B press
            __import__(app)
    except BaseException as e:
        print("Fatal exception in the running app!")
        sys.print_exception(e)
