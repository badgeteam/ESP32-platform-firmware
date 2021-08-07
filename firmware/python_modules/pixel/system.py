import machine, time, term, buttons

# Deep sleep wakeup button
pin = buttons._gpioMap[buttons.BTN_A]
machine.RTC().wake_on_ext0(pin=pin, level=0)

def clear_boot_magic():
    rtc = machine.RTC()
    rtc.write(0,0)
    rtc.write(1,0)

def reboot(goHome=True):
    if goHome:
        home()
    else:
        machine.deepsleep(1)

def sleep(duration=0, status=False):
    if (duration >= 86400000): #One day
        duration = 0
    if status:
        if duration < 1:
            term.header(True, "Sleeping until touchbutton is pressed...")
        else:
            term.header(True, "Sleeping for "+str(duration)+"ms...")
    time.sleep(0.1)
    machine.deepsleep(0)

def isColdBoot():
    if machine.wake_reason() == (7, 0):
        return True
    return False

def isWakeup(fromTimer=True,fromButton=True, fromIr=True, fromUlp=True):
    if fromButton and machine.wake_reason() == (3, 1):
        return True
    if fromIr     and machine.wake_reason() == (3, 2):
        return True
    if fromTimer  and machine.wake_reason() == (3, 4):
        return True
    if fromUlp    and machine.wake_reason() == (3, 5):
        return True
    return False

# Application launching

def start(app, status=True):
    if status:
        if app == "" or app == "launcher":
            term.header(True, "Loading menu...")
        else:
            term.header(True, "Loading application "+app+"...")
    machine.RTC().write_string(app)
    machine.deepsleep(1)

def home(status=False):
    start("", status)

def launcher(status=False):
    start("launcher", status)

def shell(status=False):
    start("shell", status)

# Over-the-air updating

def ota(status=False):
    if status:
        term.header(True, "Starting update...")
    rtc = machine.RTC()
    rtc.write(0,1)
    rtc.write(1,254)
    machine.deepsleep(2)

def serialWarning():
    pass

__current_app__ = None

def currentApp():
    return __current_app__

def get_vcc_bat():
    voltage_bat = None
    try:
        vcc_bat = machine.ADC(machine.Pin(35))
        vcc_bat.width(machine.ADC.WIDTH_12BIT)
        vcc_bat.atten(machine.ADC.ATTN_11DB)
        voltage_bat = int(vcc_bat.read() / (4095 / 4034) * 2 )
        vcc_bat.deinit()
    finally:
        return voltage_bat

def crashedWarning():
    pass
