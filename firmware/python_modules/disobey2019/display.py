import framebuf, erc12864

rawbuffer = bytearray(1024)
framebuffer = framebuf.FrameBuffer(rawbuffer, 128, 64, framebuf.MONO_VLSB)
framebuffer.fill(0)

def write():
	erc12864.write(bytes(rawbuffer))
