import virtualtimers, system, deepsleep

## Make badge sleep in undervoltage conditions
virtualtimers.activate(1000) # low resolution needed
def _vcc_callback():
    try:
        vcc = system.get_vcc_bat()
        if vcc != None:
            if vcc < 3300:
                deepsleep.vcc_low()
    finally:
        # Return 10000 to start again in 10 seconds
        return 10000

print('Starting undervoltage monitor')
virtualtimers.new(10000,_vcc_callback)