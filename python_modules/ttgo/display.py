import framebuf, ssd1306

rawbuffer = bytearray(1024)
framebuffer = framebuf.FrameBuffer(rawbuffer, 128, 64, framebuf.MONO_VLSB)
framebuffer.fill(0)

def write():
	ssd1306.write(bytes(rawbuffer))
