import term, mpr121, time, display, machine, easydraw

display.drawFill()
easydraw.msg("Do not press any buttons!", "Touch calibration", True)
time.sleep(0.5)

def setDefault():
	baseline_def = []
	for i in range(12):
		baseline_def.append(mpr121.getDefaultBaseline(i))
	mpr121.configure(baseline_def, False)

setDefault()

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
			if baseLine[i]//baseLineCount * 100 < mpr121.getDefaultBaseline(i) * 85:
				tooLow = True
			if baseLine[i]//baseLineCount * 95 > mpr121.getDefaultBaseline(i) * 100:
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
	if mpr121.getDefaultBaseline(i) > 0:
		print(i,mpr121.getDefaultBaseline(i), baseLine[i], "=", baseLine[i]-mpr121.getDefaultBaseline(i))

for i in range(12):
	if mpr121.getDefaultBaseline(i) > 0 and mpr121.get(i):
		waitRelease(i)

easydraw.msg("Check passed.")

for i in range(12):
	machine.nvs_set_u16("system", "mpr121.base."+str(i), baseLine[i])

easydraw.msg("Calibration stored to NVS!")
