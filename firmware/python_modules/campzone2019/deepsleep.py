def start_sleeping(sleepTime=0):
    import term, rgb, time, machine
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
    import term, rgb, machine
    term.header(True, "Going to sleep...")
    rgb.enablecomp()
    rgb.background((0,0,0))
    rgb.clear()
    uinterface.skippabletext('BATT LOW!')

    time.sleep(0.1)
    machine.deepsleep(sleepTime)


def reboot():
    import machine
    machine.deepsleep(1)
