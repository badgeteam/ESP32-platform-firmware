import machine

tz = machine.RTC().timezone()
print("Default timezone:", tz)

machine.nvs_setint('system', 'factory_checked', 1) # We have completed the factory script
