import time, system
import rgb
from random import random, randint
from math import sqrt
from consts import INFO_HARDWARE_WOEZEL_NAME

xdir, ydir, zdir = 1, 1, 1

if INFO_HARDWARE_WOEZEL_NAME == 'pixel':
    xdir, ydir, zdir = -1, 1, 1

try:
    import mpu6050
except:
    rgb.scrolltext('Newer firmware needed')
    time.sleep(8)
    system.reboot()

### Based on https://github.com/pierre-muth/IGG_-64x64M/blob/master/java/src/pixeldust/PixelDust.java

n_grains = 32

width, height = rgb.PANEL_WIDTH, rgb.PANEL_HEIGHT
w8 = (width+7)/8
x_max, y_max = (width-1)*256-1, (height-1)*256-1
scale = 1
elasticity = 64
sort = True

grains = []
framebuffer = [0] * width * height

class Grain:
    x = 0
    y = 0
    vx = 0
    vy = 0

    def __init__(self, x, y, vx, vy):
        self.x = x
        self.y = y
        self.vx = vx
        self.vy = vy

    def pos(self):
        return (self.x, self.y)

    def velocity(self):
        return (self.vx, self.vy)

def getpixel(pos):
    x, y = pos
    x = int(round(x/256))
    y = int(round(y/256))
    value = framebuffer[y * width + x]
    # print('Getting', x, y, y * width + x, pos[0], pos[1])
    # print('0x{:08x}'.format(value))
    return value

def setpixel(pos):
    x, y = pos
    x = int(round(x/256))
    y = int(round(y/256))
    # print('Setting', x, y, y * width + x, pos[0], pos[1])
    col_left = 0x0000FF00
    col_right = 0xFF00F000
    col_final = int((1-(float(x + 1)/width)) * col_left) + \
                int(float(x + 1)/width * col_right)
    framebuffer[y * width + x] = col_final

def clearpixel(pos):
    x, y = pos
    x = int(round(x/256))
    y = int(round(y/256))
    # print('Clearing', x, y, y * width + x, pos[0], pos[1])
    framebuffer[y * width + x] = 0

def clear():
    for i in range(0, len(framebuffer)):
        framebuffer[i] = 0

def render():
    rgb.frame(framebuffer)

def bounce(n):
    return int((-n) * elasticity / 256.0)

def init():
    for _ in range(0, n_grains):
        while True:
            x, y = randint(0, x_max), randint(0, y_max)
            if not getpixel((x,y)):
                break
        vx, vy = 0, 0  # Velocity
        grains.append(Grain(x,y, vx, vy))
        setpixel((x,y))

def noisy_get_accel():
    tries = 0
    ax, ay, az = 0.0, 0.0, 0.0
    while ax == 0.0 and ay == 0.0 and az == 0.0:
        ax, ay, az = mpu6050.get_accel()
        tries += 1
        if tries % 5 == 0:
            print('Reinitialising MPU6050')
            mpu6050.init()

    return ax*xdir, ay*ydir, az*zdir

def step():
    # mpu6050.init()
    ax, ay, az = noisy_get_accel()
    # ax, ay, az = (5000, 0, 0)
    ax *= float(scale) / 256
    ay *= -float(scale) / 256
    az *= float(scale) / 2048

    print('Accel', ax, ay, az)

    # A tiny bit of random motion is applied to each grain, so that tall
    # stacks of pixels tend to topple (else the whole stack slides across
    # the display).  This is a function of the Z axis input, so it's more
    # pronounced the more the display is tilted (else the grains shift
    # around too much when the display is held level).
    az  = 1 if (az >= 4) else 5 - az  # Clip & invert
    ax -= az                          # Subtract Z motion factor from X, Y,
    ay -= az                          # then...
    az2 = az * 2 + 1                  # max random motion to add back in

    # Apply 2D accel vector to grain velocities...
    v2 = 0  # Velocity squared
    v = 0   # Absolute velocity

    for grain in grains:
        grain.vx += ax + random()*az2  # X velocity
        grain.vy += ay + random()*az2  # Y velocity
        # Terminal velocity (in any direction) is 256 units -- equal to
        # 1 pixel -- which keeps moving grains from passing through each other
        # and other such mayhem.  Though it takes some extra math, velocity is
        # clipped as a 2D vector (not separately-limited X & Y) so that
        # diagonal movement isn't faster than horizontal/vertical.
        v2 = grain.vx*grain.vx+grain.vy*grain.vy
        if v2 > 65536:  # If v^2 > 65536, then v > 256
            v = sqrt(v2)
            grain.vx = int(256.0*float(grain.vx/v))  # Maintain heading &
            grain.vy = int(256.0*float(grain.vy/v))  # limit magnitude

    # ...then update position of each grain, one at a time, checking for
    # collisions and having them react.  This really seems like it shouldn't
    # work, as only one grain is considered at a time while the rest are
    # regarded as stationary.  Yet this naive algorithm, taking many not-
    # technically-quite-correct steps, and repeated quickly enough,
    # visually integrates into something that somewhat resembles physics.
    # (I'd initially tried implementing this as a bunch of concurrent and
    # "realistic" elastic collisions among circular grains, but the
    # calculations and volume of code quickly got out of hand for both
    # the tiny 8-bit AVR microcontroller and my tiny dinosaur brain.)

    for grain_index, grain in enumerate(grains):
        oldpos, velocity = grain.pos(), grain.velocity()
        x, y = oldpos
        vx, vy = velocity
        newx, newy = x + vx, y + vy

        if newx < 0:
            newx = 0
            grain.vx = bounce(vx)
        elif newx > x_max:
            newx = x_max
            grain.vx = bounce(vx)

        if newy < 0:
            newy = 0
            grain.vy = bounce(vy)
        elif newy > y_max:
            newy = y_max
            grain.vy = bounce(vy)

        # oldpos/newpos are the prior and new pixel index for this grain.
        # It's a little easier to check motion vs handling X & Y separately.
        newpos = (newx, newy)
        oldindex = int(int(round(y / 256)) * width + int(round(x / 256)))
        newindex = int(int(round(newy / 256)) * width + int(round(newx / 256)))
        if oldindex != newindex and getpixel(newpos):  # If grain is moving to a new pixel but already occupied..
            pass
            delta = abs(newindex - oldindex)  # What direction when blocked?
            if delta == 1:  # 1 pixel left or right
                # print('d1')
                newx = grain.x
                grain.vx = bounce(grain.vx)
            elif delta == width:  # 1 pixel up or down
                # print('dw')
                newy = grain.y
                grain.vy = bounce(grain.vy)
            else:  # Diagonal intersection is more tricky...
                # Try skidding along just one axis of motion if possible
                # (start w/faster axis).
                if abs(grain.vx) > abs(grain.vy):  # X axis is faster
                    if not getpixel((newx, grain.y)):
                        # print('xff')
                        # That pixel's free!  Take it!  But...
                        newy = grain.y  # Cancel Y motion
                        grain.vy = bounce(grain.vy)  # and bounce Y velocity
                    else:
                        # X pixel is taken, so try Y...
                        if not getpixel((grain.x, newy)):
                            # print('xfy')
                            # That pixel's free!  Take it!  But...
                            newx = grain.x  # Cancel X motion
                            grain.vx = bounce(grain.vx)  # and bounce X velocity
                        else:
                            # print('xfb')
                            # Both spots are occupied
                            newx = grain.x  # Cancel X & Y motion
                            newy = grain.y
                            grain.vx = bounce(grain.vx)  # Bounce X & Y velocity
                            grain.vy = bounce(grain.vy)
                else:  # Y axis is faster
                    if not getpixel((grain.x, newy)):
                        # print('yff')
                        # That pixel's free!  Take it!  But...
                        newx = grain.x  # Cancel X motion
                        grain.vx = bounce(grain.vx)  # and bounce X velocity
                    else:
                        # Y pixel is taken, so try X...
                        if not getpixel((newx, grain.y)):
                            # print('yfx')
                            # That pixel's free!  Take it!  But...
                            newy = grain.y  # Cancel Y motion
                            grain.vy = bounce(grain.vy)  # and bounce Y velocity
                        else:
                            # print('yfb')
                            # Both spots are occupied
                            newx = grain.x  # Cancel X & Y motion
                            newy = grain.y
                            grain.vx = bounce(grain.vx)  # Bounce X & Y velocity
                            grain.vy = bounce(grain.vy)
        else:
            # print('dobby is free!')
            pass

        clearpixel(oldpos)
        grain.x = newx
        grain.y = newy
        setpixel((newx, newy))
        # print('Setting', grain_index, 'from', oldpos, 'to', newpos)

if not mpu6050.has_sensor():
    rgb.scrolltext('Addon required')
    time.sleep(6)
    system.reboot()

init()
mpu6050.init()
rgb.disablecomp()
while True:
    step()
    render()