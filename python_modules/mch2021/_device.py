import os, machine, display, mascot, easydraw, time

PIN_PWR_SDCARD  = 5
PIN_IRQ_BUTTONS = 34
PIN_IRQ_ACCEL   = 36
PIN_IRQ_FPGA    = 39

sdcardPower = machine.Pin(PIN_PWR_SDCARD, machine.Pin.OUT)


def configureWakeupSource():
    machine.RTC().wake_on_ext0(
        pin = machine.Pin(PIN_IRQ_BUTTONS),
        level = 0
    )
    machine.RTC().wake_on_ext1([
        machine.Pin(PIN_IRQ_ACCEL, machine.Pin.IN),
        machine.Pin(PIN_IRQ_FPGA,  machine.Pin.IN)
    ], 0)
    return True

def prepareForSleep():
    try:
        os.umountsd()
    except:
        pass
    sdcardPower.value(False)
    configureWakeupSource()

def prepareForWakeup():
    sdcardPower.value(True)
    time.sleep(0.05) # Give the SD card time to initialize itself
    os.mountsd()

def showLoadingScreen(app=""):
    try:
        display.drawFill(0x000000)
        display.drawPng(85, 13, mascot.logo)
        display.drawText(0, display.height() - 18, app, 0xFFFF00, "ocra16")
        display.flush()
    except:
        pass

def showMessage(message="", icon=None):
    easydraw.messageCentered(message, False, icon)

def setLedPower(state):
    pass
