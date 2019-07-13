import time, ntp, hub75, easywifi, buttons, defines, system

direction = 0

if not easywifi.status():
    easywifi.enable()

if not ntp.set_NTP_time():
    print("Error setting time")
    quit()

def input_B(pressed):
    global direction
    direction = defines.BTN_B

buttons.register(defines.BTN_B, input_B)
hub75.background(0,0,0)
hub75.clear()

gifd = [0, 0x00FFFFFF, 0, 0, 0, 0x00FFFFFF, 0, 0, 0, 0, 0, 0, 0, 0]

tmold = 70
hub75.framerate(1)
while direction != defines.BTN_B:
    th = time.strftime("%H")
    tm = time.strftime("%M")
    if tm != tmold:
        hub75.clear()
        hub75.text(th, 255, 255, 255, 3, 0)
        hub75.text(tm, 255, 255, 255, 18, 0)
        hub75.gif(gifd, 15, 0, 1, 7, 2)
        tmold = tm
system.reboot()