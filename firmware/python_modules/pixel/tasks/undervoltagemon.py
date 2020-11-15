# import virtualtimers, system, gc

## This functionality is now in _boot.py, because for some reason
## simply importing this file in the boot process costs 10k RAM.

## Make badge sleep in undervoltage conditions
# virtualtimers.activate(1000) # low resolution needed
# def _vcc_callback():
#     print('vcccb')
#     try:
#         vcc = system.get_vcc_bat()
#         if vcc != None:
#             if vcc < 3300:
#                 __import__('deepsleep')
#                 deepsleep.vcc_low()
#     finally:
#         # Return 10000 to start again in 10 seconds
#         gc.collect()
#         return 10000

# virtualtimers.new(10000, _vcc_callback, hfpm=True)