import framebuf, eink

rawbuffer = bytearray(int(296*128/8))
framebuffer = framebuf.FrameBuffer(rawbuffer, 296, 128, framebuf.MONO_HMSB)
framebuffer.fill(255)

BLACK = 0
WHITE = 1

def write(flags=0):
	eink.write(bytes(rawbuffer),flags)
