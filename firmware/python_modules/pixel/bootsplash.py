import rgb, time, system

for _ in range(2):
	for x in range(31):
		rgb.disablecomp()
		rgb.clear()
		rgb.pixel(pos=(x, 0))
		rgb.pixel(pos=(x+1, 0))
		rgb.pixel(pos=(30-x, 7))
		rgb.pixel(pos=(31-x, 7))
		rgb.enablecomp()
		time.sleep(0.05)

	rgb.disablecomp()
	rgb.clear()
	rgb.pixel(pos=(31, 0))
	rgb.pixel(pos=(0, 7))
	rgb.enablecomp()
	time.sleep(0.05)

	for y in range(7):
		rgb.disablecomp()
		rgb.clear()
		rgb.pixel(pos=(31, y))
		rgb.pixel(pos=(31, y+1))
		rgb.pixel(pos=(0, 6-y))
		rgb.pixel(pos=(0, 7-y))
		rgb.enablecomp()
		time.sleep(0.05)

	rgb.disablecomp()
	rgb.clear()
	rgb.pixel(pos=(31, 7))
	rgb.pixel(pos=(0, 0))
	rgb.enablecomp()
	time.sleep(0.05)

rgb.clear()
time.sleep(0.5)

# for brightness in range(64, 255):
# 	rgb.disablecomp()
# 	rgb.text('Pixel', color=(brightness,brightness,brightness), pos=(2,0))
# 	rgb.enablecomp()
# 	time.sleep(0.05)

import nyan

time.sleep(5)
system.launcher()