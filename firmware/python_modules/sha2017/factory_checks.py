import machine, display, time, system

tz = machine.RTC().timezone()
print("Default timezone:", tz)

machine.nvs_setint('system', 'factory_checked', 1) # We have completed the factory script

display.fill(0xFFFFFF)
display.drawText(0,0,"Welcome to the BADGE.TEAM platform firmware!", 0x000000, "7x5")

time.sleep(2)

system.home()
