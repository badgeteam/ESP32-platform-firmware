# Device specific system functions: SHA2017
import mpr121, eink, easydraw, display, time, os, machine

def configureWakeupSource():
    machine.RTC().wake_on_ext0(pin = machine.Pin(25), level = 0)
    machine.RTC().wake_on_ext1([machine.Pin(12, machine.Pin.IN, machine.Pin.PULL_UP)], 0)
    return True

def prepareForSleep():
    configureWakeupSource()
    try:
        os.umountsd()
    except:
        pass
    mpr121.set(10, False)
    eink.busy_wait()

def prepareForWakeup():
    mpr121.set(10, True)
    time.sleep(0.05) # Give the SD card time to initialize itself
    os.mountsd()

def showLoadingScreen(app=""):
    try:
        display.drawFill(0xFFFFFF)
        text = "Loading...\n\n{}".format(app)
        display.drawText((display.width()-display.getTextWidth(text, "ocra16"))//2, (display.height()-display.getTextHeight(text, "ocra16"))//2, text, 0x000000, "ocra16")
        display.flush(display.FLAG_LUT_FASTEST)
    except:
        pass

def showMessage(message="", icon=None):
    easydraw.messageCentered(message, False, icon)
