import term, mpr121, time, display, machine, easydraw, _mpr121mapping

try:
	import buttons
	buttons.detach(buttons.BTN_START)
except:
	pass

pinToName = ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"]

try:
	pinToName = _mpr121mapping.pinToName
except:
	pass

try:
	correction_factor = _mpr121mapping.correction_factor
except:
	correction_factor = 1

easydraw.messageCentered("CALIBRATION\nDo not press any buttons!")
time.sleep(1)

# ----

try:
	verification = _mpr121mapping.verification
	use_verification = True
except:
	verification = [0,0,0,0,0,0,0,0,0,0,0,0]
	use_verification = False
# ----

mpr121.configure([], 24, 12) # Automatic detection

baseLine = [0]*12
isPressed = [False]*12
isTooHigh = [False]*12
isTooLow = [False]*12
baseLineCount = 0
isTouch = []

for i in range(12):
	isTouch.append(mpr121.isTouch(i))

avAmount = 16
easydraw.messageCentered("CALIBRATION\nCalibrating...")
#print("Collecting samples...")

for i in range(avAmount):
	#print("Collecting samples... ({} of {})".format(i+1, avAmount))
	time.sleep(0.1)
	info = mpr121.touchInfo()
	baseLineCount += 1
	for i in range(len(info)):
		if isTouch[i]:
			baseLine[i] += info[i]['data'] >> 2
			tooLow = False
			tooHigh = False
			if baseLine[i]//baseLineCount * 100 < verification[i] * 85:
				tooLow = True
			if baseLine[i]//baseLineCount * 95 > verification[i] * 100:
				tooHigh = True
			isTooLow[i] = tooLow
			isTooHigh[i] = tooHigh

for i in range(12):
	baseLine[i] = baseLine[i] // avAmount

print("")
print("Port\t Verification \t Measurement \t Low \t High \t Difference");
print("==================================================================")
for i in range(12):
	print(i,"\t",verification[i],"\t\t",baseLine[i], "\t\t", isTooLow[i], "\t", isTooHigh[i],"\t", baseLine[i] - verification[i])
print("")

for i in range(12):
	baseLine[i] = int(baseLine[i]*correction_factor)

mpr121.configure(baseLine)

def waitTouch(i,state):
	easydraw.messageCentered("ACTION\nPress "+pinToName[i]+"\n("+state+")")
	print("("+state+") ACTION: Press "+pinToName[i])
	while not mpr121.get(i):
		time.sleep(0.01)
	print("OK")
	
def waitRelease(i,state):
	easydraw.messageCentered("ACTION\nRelease "+pinToName[i]+"\n("+state+")")
	print("("+state+") ACTION: Release "+pinToName[i])
	while mpr121.get(i):
		time.sleep(0.01)
	print("OK")

if use_verification:
	easydraw.messageCentered("CALIBRATION\nVerification...")
	for i in range(12):
		if not isTouch[i]:
			continue
		elif isTooLow[i]:
			#easydraw.msg("(low)")
			waitTouch(i, "too low")
			waitRelease(i, "too low")
		elif isTooHigh[i]:
			#easydraw.msg("(high)")
			waitTouch(i, "too high")
			waitRelease(i, "too high")

easydraw.messageCentered("CALIBRATION\nWaiting for release...")

for i in range(12):
	if mpr121.isTouch(i) and mpr121.get(i):
		waitRelease(i,"check")

easydraw.messageCentered("CALIBRATION\nSaving to NVS...")

for i in range(12):
	machine.nvs_set_u16("system", "mpr121.base."+str(i), baseLine[i])

easydraw.messageCentered("CALIBRATION\nOK")
