import machine, term, time, defines, rgb, buttons

pin = buttons._gpioMap[buttons.BTN_A]
rtc = machine.RTC()
rtc.wake_on_ext0(pin=pin, level=0)

def start_sleeping(sleepTime=0):
    term.header(True, "Going to sleep...")

    rgb.clear()
    rgb.scrolltext('ZzZz')
    time.sleep(3)

    if (sleepTime >= 86400000):  # One day
        sleepTime = 0
    if (sleepTime < 1):
        print("Sleeping until A-button is pressed...")
    else:
        print("Sleeping for " + str(sleepTime) + "ms...")
    time.sleep(0.1)
    machine.deepsleep(sleepTime)

def vcc_low(sleepTime=0):
    term.header(True, "Going to sleep...")
    rgb.enablecomp()
    rgb.background((0,0,0))
    rgb.clear()
    rgb.scrolltext('BATT LOW!')
    time.sleep(4)

    time.sleep(0.1)
    machine.deepsleep(sleepTime)


def reboot():
    machine.deepsleep(1)
