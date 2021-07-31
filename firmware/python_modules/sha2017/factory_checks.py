import machine, display, time, system

# SHA2017 "factory" tool
# Does function as factory tool but also implements an upgrade path from our old firmware

currentState = machine.nvs_getint('system', 'factory_checked') or 0

if currentState < 2:
    display.drawFill(0xFFFFFF)
    display.drawText(0,0,"Welcome to the BADGE.TEAM platform firmware!", 0x000000, "7x5")
    display.drawText(0,6,"Please wait while we're setting things up...", 0x000000, "7x5")
    display.flush()
    time.sleep(2)

    # Check if we have upgraded from a legacy firmware
    legacy_mpr0 = machine.nvs_get_u16("badge", "mpr121.base.0")
    if legacy_mpr0:
        display.drawFill(0xFFFFFF)
        display.drawText(0,0,"Welcome to the BADGE.TEAM platform firmware!", 0x000000, "7x5")
        display.drawText(0,6,"You have upgraded from an older firmware,", 0x000000, "7x5")
        display.drawText(0,12,"now migrating your settings...", 0x000000, "7x5")
        display.flush()
            
        legacy_wifi_ssid = machine.nvs_getstr("badge", "wifi.ssid")
        legacy_wifi_password = machine.nvs_getstr("badge", "wifi.password")
        legacy_eink_type = machine.nvs_getstr("badge", "eink.dev.type")
        
        if currentState != 1:
            print("Badge was upgraded from 2017 firmware. Move WiFi settings...")
            if legacy_wifi_ssid:
                machine.nvs_setstr("system", "wifi.ssid", legacy_wifi_ssid)
            if legacy_wifi_password:
                machine.nvs_setstr("system", "wifi.password", legacy_wifi_password)
            if legacy_eink_type:
                machine.nvs_setstr("system", "eink.dev.type", legacy_eink_type)
        else:
            print("Badge was upgraded from a very early version of the platform firmware.")

        try:
            legacy_mpr1 = machine.nvs_get_u16("badge", "mpr121.base.1")
            legacy_mpr2 = machine.nvs_get_u16("badge", "mpr121.base.2")
            legacy_mpr3 = machine.nvs_get_u16("badge", "mpr121.base.3")
            legacy_mpr4 = machine.nvs_get_u16("badge", "mpr121.base.4")
            legacy_mpr5 = machine.nvs_get_u16("badge", "mpr121.base.5")
            legacy_mpr6 = machine.nvs_get_u16("badge", "mpr121.base.6")
            legacy_mpr7 = machine.nvs_get_u16("badge", "mpr121.base.7")
            machine.nvs_set_u16("system", "mpr121.base.0", legacy_mpr0)
            machine.nvs_set_u16("system", "mpr121.base.1", legacy_mpr1)
            machine.nvs_set_u16("system", "mpr121.base.2", legacy_mpr2)
            machine.nvs_set_u16("system", "mpr121.base.3", legacy_mpr3)
            machine.nvs_set_u16("system", "mpr121.base.4", legacy_mpr4)
            machine.nvs_set_u16("system", "mpr121.base.5", legacy_mpr5)
            machine.nvs_set_u16("system", "mpr121.base.6", legacy_mpr6)
            machine.nvs_set_u16("system", "mpr121.base.7", legacy_mpr7)
        except:
            print("Unable to move MPR121 calibration!")
            import _mpr121calib
    else:
        print("Badge has been freshly installed!")
        import _mpr121calib	
    try:
        # Remove old settings from NVS
        machine.nvs_erase_all("badge")
    except:
        pass

if currentState < 3:
	import dashboard.resources.png_icons as icons
	icons.install()

	
# We have completed the factory script
machine.nvs_setint('system', 'factory_checked', 3)
system.home()
