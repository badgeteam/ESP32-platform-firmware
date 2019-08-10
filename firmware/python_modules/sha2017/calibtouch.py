import term, mpr121, time, display, machine, easydraw

display.drawFill()
easydraw.msg("Do not press any buttons!", "Touch calibration", True)
time.sleep(0.5)

# These values were measured on a known-good badge and are used as the starting point for the calibration procedure
baseline_def = [ 0x0138, 0x0144, 0x0170, 0x0174, 0x00f0, 0x0103, 0x00ff, 0x00ed, 0,0,0,0 ] #SHA2017
#baseline_def = [ 350, 361, 343, 252, 267, 258, 232, 352, 0, 0, 0] #HackerHotel2019

mpr121.configure(baseline_def, False)

baseLine = [0]*12
isPressed = [False]*12
isTooHigh = [False]*12
isTooLow = [False]*12
baseLineCount = 0
isTouch = []

for i in range(12):
	isTouch.append(mpr121.isTouch(i))

avAmount = 32
easydraw.msg("Collecting samples...")

for i in range(avAmount):
	print("Collecting samples... ({} of {})".format(i+1, avAmount))
	time.sleep(0.1)
	info = mpr121.touchInfo()
	baseLineCount += 1
	for i in range(len(info)):
		if isTouch[i]:
			baseLine[i] += info[i]['data']
			tooLow = False
			tooHigh = False
			if baseLine[i]//baseLineCount * 100 < baseline_def[i] * 85:
				tooLow = True
			if baseLine[i]//baseLineCount * 95 > baseline_def[i] * 100:
				tooHigh = True
			isTooLow[i] = tooLow
			isTooHigh[i] = tooHigh

easydraw.msg("Verifying...")

for i in range(12):
	baseLine[i] = baseLine[i] // avAmount

mpr121.configure(baseLine, True)

def waitTouch(i):
	easydraw.msg("ACTION: Press button "+str(i))
	while not mpr121.get(i):
		pass
	
def waitRelease(i):
	easydraw.msg("ACTION: Release button "+str(i))
	while mpr121.get(i):
		pass

for i in range(12):
	if not isTouch[i]:
		continue
	elif isTooLow[i]:
		easydraw.msg(str(i)+". Too low!")
		waitTouch(i)
		waitRelease(i)
	elif isTooHigh[i]:
		easydraw.msg(str(i)+". Too high!")
		waitTouch(i)
		waitRelease(i)

for i in range(12):
	if baseline_def[i] > 0:
		print(i,baseline_def[i], baseLine[i], "=", baseLine[i]-baseline_def[i])

for i in range(12):
	if baseline_def[i] > 0 and mpr121.get(i):
		waitRelease(i)

easydraw.msg("Check passed.")

for i in range(12):
	machine.nvs_set_u16("system", "mpr121.base."+str(i), baseLine[i])

easydraw.msg("Calibration stored to NVS!")
