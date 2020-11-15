import machine, virtualtimers, rgb, hub75

vusb = machine.ADC(machine.Pin(34))
vusb.width(machine.ADC.WIDTH_10BIT)
vusb.atten(machine.ADC.ATTN_11DB)

import time
while True:
    rgb.clear()
    rgb.scrolltext(str(vusb.read()*2/1000.0))
    time.sleep(1)

vbat = machine.ADC(machine.Pin(35))
vbat.width(machine.ADC.WIDTH_10BIT)
vbat.atten(machine.ADC.ATTN_11DB)


samples = 500
duty = 0
# en = machine.PWM(12, freq=20000)
# en.duty(duty)


def curvolt():
    global samples, duty

    if samples >= 200:
        samples = 0
        rgb.clear()
        rgb.scrolltext('Powertest')

    intensity = hub75.image()
    brightness = rgb.getbrightness()
    deduced_current = float(intensity) / 255 * 0.02 * ((brightness-2) / 32) / 8  # in Amperes

    needed_duty = 100
    if deduced_current <= 0.25:
        needed_duty = int(deduced_current / 0.25 * 95) + 5

    # en.duty(needed_duty)

    print('%d,%d,%.3f,%.3f,%.3f' % (samples, needed_duty, vusb.read()*2/1000.0, vbat.read()*2/1000.0, deduced_current))

    samples += 1
    return 100

virtualtimers.activate(100)
virtualtimers.new(100, curvolt)