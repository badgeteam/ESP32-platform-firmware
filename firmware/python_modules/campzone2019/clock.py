import time, ntp, rgb, wifi, buttons, defines, system

direction = 0

if not wifi.status():
    wifi.connect()

if not ntp.set_NTP_time():
    print("Error setting time")
    quit()

wifi.disconnect()

def input_B(pressed):
    global direction
    direction = defines.BTN_B

buttons.register(defines.BTN_B, input_B)

gifd = [0, 0x00FFFFFF, 0, 0, 0, 0x00FFFFFF, 0, 0, 0, 0, 0, 0, 0, 0]

tmold = 70
rgb.framerate(1)

while direction != defines.BTN_B:
    th = time.strftime("%H")
    tm = time.strftime("%M")
    if tm != tmold:
        hub75.clear()
        hub75.text(th, 255, 255, 255, 3, 0)
        hub75.text(tm, 255, 255, 255, 18, 0)
        hub75.gif(gifd, 15, 0, 1, 7, 2)
        tmold = tm
    # rgb.disablecomp()
    # rgb.clear()
    # rgb.text(time.strftime("%H:%M:%S"),  (255, 255, 255), (3,2))
    # rgb.enablecomp()
    time.sleep(0.2)

system.reboot()