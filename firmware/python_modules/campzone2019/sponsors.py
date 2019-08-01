import rgb, time, system, uinterface

rgb.clear()
rgb.setfont(rgb.FONT_7x5)

uinterface.skippabletext("Your badge was sponsored by:")
rgb.clear()
time.sleep(1)

uinterface.skippabletext("Espressif")
rgb.clear()
time.sleep(1)

uinterface.skippabletext("AllNet")
rgb.clear()
time.sleep(1)

uinterface.skippabletext("and")
rgb.clear()
time.sleep(1)

rgb.setfont(rgb.FONT_6x3)

for i in range(1, 60):
    intensity = int(float(255) / 60 * i)
    rgb.disablecomp()
    rgb.clear()
    rgb.text("Deloitte", (intensity,intensity,intensity), (1, 1))
    rgb.enablecomp()
    time.sleep(0.03)

time.sleep(2)
rgb.pixel((114, 159, 30), (29, 4))
rgb.pixel((114, 159, 30), (29, 5))
rgb.pixel((114, 159, 30), (30, 4))
rgb.pixel((114, 159, 30), (30, 5))
time.sleep(3)