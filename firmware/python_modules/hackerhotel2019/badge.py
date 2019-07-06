import mpr121

def onBtnA(cb=None):
	mpr121.attach(0, cb)

def onBtnB(cb=None):
	mpr121.attach(1, cb)

def onBtnStart(cb=None):
	mpr121.attach(2, cb)

def onBtnSelect(cb=None):
	mpr121.attach(3, cb)

def onBtnDown(cb=None):
	mpr121.attach(4, cb)

def onBtnRight(cb=None):
	mpr121.attach(5, cb)

def onBtnUp(cb=None):
	mpr121.attach(6, cb)

def onBtnLeft(cb=None):
	mpr121.attach(7, cb)

def onChrgStatusChange(cb=None):
	mpr121.attach(9, cb)

def readBtnA():
	return mpr121.get(0)

def readBtnB():
	return mpr121.get(1)

def readBtnStart():
	return mpr121.get(2)

def readBtnSelect():
	return mpr121.get(3)

def readBtnDown():
	return mpr121.get(4)

def readBtnRight():
	return mpr121.get(5)

def readBtnUp():
	return mpr121.get(6)

def readBtnLeft():
	return mpr121.get(7)

def readChrgStatus():
	return mpr121.get(9)

def setVibrator(state):
	mpr121.set(8, state)

def setPower(state):
	mpr121.set(10, state)

