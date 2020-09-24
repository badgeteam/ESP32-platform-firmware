import machine, system

##
# Add any hardware checks needed to test the device after initial flashing.
# This runs only once, and disables itself afterwards with the following line:
machine.nvs_setint('system', 'factory_checked', 2)

system.reboot()