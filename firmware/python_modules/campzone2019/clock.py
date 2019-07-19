import time, ntp, rgb, wifi, buttons, defines, system

direction = 0

if not wifi.status():
    wifi.connect()

if not wifi.status():
    print("Error connecting to wifi")
    system.reboot()

if not ntp.set_NTP_time():
    print("Error setting time")
    system.reboot()

wifi.disconnect()

def input_B(pressed):
    global direction
    direction = defines.BTN_B

buttons.register(defines.BTN_B, input_B)

gifd = [0, 0x00FFFFFF, 0, 0, 0, 0x00FFFFFF, 0, 0, 0, 0, 0, 0, 0, 0]

tmold = 70
rgb.background(0,0,0)
rgb.clear()
rgb.framerate(1)

while direction != defines.BTN_B:
    th = time.strftime("%H")
    tm = time.strftime("%M")
    if tm != tmold:
        hub75.clear()
        hub75.text(th, 255, 255, 255, 3, 0)
        hub75.text(tm, 255, 255, 255, 18, 0)
        hub75.gif(gifd, 15, 0, 1, 7, 2)
        hub75.gif(gifd, 16, 0, 1, 7, 2)
        tmold = tm
    time.sleep(0.2)
system.reboot()