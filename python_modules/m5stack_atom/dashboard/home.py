# Poorly written quick'n dirty demo for the M5STACK ATOM

import mpu6050 as mpu, neopixel, time
import _buttons as btn

import virtualtimers

btn.register(39)

mode = 3

def btnCb(pressed):
	global mode
	if pressed:
		mode += 1
		if mode > 3:
			mode = 0

btn.attach(39, btnCb)

def setLed(l,r,u,d,z1,z2,gl,gr,gu,gd,gz1,gz2):
	data = [0x00, 0x00, 0x00]*25
	for i in range(3):
		data[(i+21)*3] = l
		data[(i+1)*3] = r
		data[((i+1)*5)*3] = u
		data[(((i+1)*5)+4)*3] = d
		
	data[12*3+1] = z1
	data[12*3+2] = z2
	
	data[7*3+1] = gd
	data[17*3+1] = gu
	
	data[11*3+1] = gl
	data[13*3+1] = gr
	
	data[0*3+1] = gz1
	data[4*3+1] = gz1
	data[20*3+1] = gz1
	data[24*3+1] = gz1
	data[0*3+2] = gz2
	data[4*3+2] = gz2
	data[20*3+2] = gz2
	data[24*3+2] = gz2
		
	neopixel.send(bytes(data))
	
def setSmiley(r,g):
	data = [0x00, 0x00, 0x00]*25

	data[5*3+0] = g
	data[5*3+1] = r
	data[15*3+0] = g
	data[15*3+1] = r

	data[8*3+0] = g
	data[8*3+1] = r
	data[13*3+0] = g
	data[13*3+1] = r
	data[18*3+0] = g
	data[18*3+1] = r

	data[2*3+0] = g
	data[4*3+1] = r
	
	data[22*3+0] = g
	data[24*3+1] = r
	
	
	
	neopixel.send(bytes(data))


def task():
	accel = mpu.acceleration()
	gyros = mpu.gyroscope()
	
	u = 0
	d = 0
	l = 0
	r = 0
	z1 = 0
	z2 = 0
	gu = 0
	gd = 0
	gl = 0
	gr = 0
	gz1 = 0
	gz2 = 0
	
	if mode & 1:
		if (accel[0] < 0):
			u = (-accel[0])>>8
		else:
			d = (accel[0])>>8
		if (accel[1] < 0):
			r = (-accel[1])>>8
		else:
			l = (accel[1])>>8
		if (accel[2] < 0):
			z2 = (-accel[2])>>8
		else:
			z1 = (accel[2])>>8
	
	if mode & 2:
		if (gyros[0] < 0):
			gu = (-gyros[0])>>8
		else:
			gd = (gyros[0])>>8
		if (gyros[1] < 0):
			gl = (-gyros[1])>>8
		else:
			gr = (gyros[1])>>8
		if (gyros[2] < 0):
			gz1 = (-gyros[2])>>8
		else:
			gz2 = (gyros[2])>>8
	
	if mode > 0:
		setLed(l,r,u,d,z1,z2,gl,gr,gu,gd,gz1,gz2)
	else:
		r = 0
		g = 0
		if (accel[1] < 0):
			g = (-accel[1])>>8
		else:
			r = (accel[1])>>8
		setSmiley(r,g)
	
	return 20

virtualtimers.activate(20)
virtualtimers.new(20, task)

import menu
