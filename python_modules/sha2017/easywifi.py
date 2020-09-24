import time, network, badge, easydraw

state = False
failed = False

nw = network.WLAN(network.STA_IF)

def status():
    global state
    return state

def failure():
    global failed
    return failed

def force_enable():
    global state
    state = False
    global failed
    failed = False
    enable()

def enable(showStatus=True):
    global failed, state, nw
    if not state:
        if not nw.isconnected():
            nw.active(True)
            ssid = badge.nvs_get_str('system', 'wifi.ssid', 'hackerhotel-insecure')
            password = badge.nvs_get_str('system', 'wifi.password')
            if showStatus:
                easydraw.msg("Connecting to '"+ssid+"'...")
            nw.connect(ssid, password) if password else nw.connect(ssid)
            timeout = badge.nvs_get_u8('system', 'wifi.timeout', 5)
            while not nw.isconnected():
                time.sleep(1)
                timeout = timeout - 1
                if (timeout<1):
                    if showStatus:
                        easydraw.msg("Error: could not connect!")
                    disable()
                    failed = True
                    return False
            state = True
            failed = False
            if showStatus:
                easydraw.msg("Connected!")
    return True

def disable():
    global state, failed, nw
    state = False
    failed = False
    nw.active(False)
