import machine, display, time, system

tz = machine.RTC().timezone()
print("Default timezone:", tz)

machine.nvs_setint('system', 'factory_checked', 1) # We have completed the factory script

display.drawFill(0xFFFFFF)
display.drawText(0,0,"Welcome to the BADGE.TEAM platform firmware!", 0x000000, "7x5")
display.drawText(0,6,"Please wait while we're setting things up...", 0x000000, "7x5")
display.flush()

time.sleep(5)

system.home()
