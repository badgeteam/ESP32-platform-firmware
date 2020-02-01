import math, neopixel, time, system
from machine import Pin

# Play with these to change your rainbow!
hue_radius = 360
speed_multiplier = 1  # Divide by this value to get the speed the animation plays.
screen_rects = 20  # How many rectangles to use for the screen rainbow

# 0 - 11: Front
# 12 - 15: Halo
led_count = 16
row_count = 4

def hsv_2_rgb(hsv):
    h,s,v = hsv
    h = float(h)
    s = float(s)
    v = float(v)
    h60 = h / 60.0
    h60_floor = math.floor(h60)
    hi = int(h60_floor) % 6
    f = h60 - h60_floor
    p = v * (1 - s)
    q = v * (1 - f * s)
    t = v * (1 - (1 - f) * s)
    r, g, b = 0, 0, 0
    if hi == 0: r, g, b = v, t, p
    elif hi == 1: r, g, b = q, v, p
    elif hi == 2: r, g, b = p, v, t
    elif hi == 3: r, g, b = p, q, v
    elif hi == 4: r, g, b = t, p, v
    elif hi == 5: r, g, b = v, p, q
    r, g, b = int(r * 255), int(g * 255), int(b * 255)
    return r, g, b

def hue(h):
    return (h % 360, 1, 1)

def rgb_to_bytestring(rgb):
    r, g, b = rgb
    return bytes([g, r, b])

t = 0.0
last_pressed = 0

row1 = Pin(16, Pin.OUT)
col1 = Pin(36, Pin.IN)
row1.value(1)

while True:
    t += speed_multiplier
    outputs = b''
    for col in range(led_count / row_count):
        for row in range(row_count):
            outputs += rgb_to_bytestring(hsv_2_rgb(hue(int(t) + (col + row) * (hue_radius / led_count))))

    neopixel.send(outputs)

    if col1.value():
        neopixel.send(bytes([0, 0, 0]) * led_count)
        system.reboot()

    time.sleep(0.05)