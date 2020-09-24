import utime
from machine import Pin, SPI
from bmp import BitmapHeader, BitmapHeaderInfo

# Display resolution
EPD_WIDTH		  = 128
EPD_VISABLE_WIDTH  = 122
EPD_HEIGHT		 = 250

# Color or no color
COLORED = 1
UNCOLORED = 0

# Display orientation
ROTATE_0									= 0
ROTATE_90								   = 1
ROTATE_180								  = 2
ROTATE_270								  = 3
         

class EPD:
	def __init__(self, reset, dc, busy, cs):
		self.reset_pin = reset
		self.dc_pin = dc
		self.busy_pin = busy
		self.cs_pin = cs

		self.spi = SPI(1, baudrate=1000000, mosi=Pin(23, Pin.OUT), sck=Pin(18, Pin.OUT), miso=Pin(19, Pin.IN))

		self.width = EPD_WIDTH
		self.height = EPD_HEIGHT
		self.rotate = ROTATE_0

	def __del__(self):
		self.spi.deinit()

	def free(self):
		self.spi.deinit()

	def init(self):
		self.reset()

		# GDOControl
		self.send_command(0x01)
		self.send_data(249)
		self.send_data(0)
		self.send_data(0x00)

		# softstart
		self.send_command(0x0c)
		self.send_data(0xd7)
		self.send_data(0xd6)
		self.send_data(0x9d)

		# VCOMVol
		self.send_command(0x2c)
		self.send_data(0xa8)

		# DummyLine
		self.send_command(0x3a)
		self.send_data(0x1a)

		#Gatetime
		self.send_command(0x3b)
		self.send_data(0x08)

		#setRamDataEntryMode
		self.send_command(0x11)
		self.send_data(0x01)

		#_SetRamArea	
		self.send_command(0x44)
		self.send_data(0x00)
		self.send_data(0x0f)
		self.send_command(0x45)
		self.send_data(249)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)

		#_SetRamPointer
		self.send_command(0x4e)
		self.send_data(0x00)
		self.send_command(0x4f)
		self.send_data(249)
		self.send_data(0x00)

		#LUTDefault_full
		self.send_command(0x32)
		self.send_data(0x22)
		self.send_data(0x55)
		self.send_data(0xAA)
		self.send_data(0x55)
		self.send_data(0xAA)
		self.send_data(0x55)
		self.send_data(0xAA)
		self.send_data(0x11)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x1E)
		self.send_data(0x1E)
		self.send_data(0x1E)
		self.send_data(0x1E)
		self.send_data(0x1E)
		self.send_data(0x1E)
		self.send_data(0x1E)
		self.send_data(0x1E)
		self.send_data(0x01)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)

		#PowerOn
		self.send_command(0x22)
		self.send_data(0xc0)
		self.send_command(0x20)
		self.wait_until_idle()

		return 0

	def initPart(self):
		self.reset()

		# GDOControl
		self.send_command(0x01)
		self.send_data(249)
		self.send_data(0)
		self.send_data(0x00)

		# softstart
		self.send_command(0x0c)
		self.send_data(0xd7)
		self.send_data(0xd6)
		self.send_data(0x9d)

		# VCOMVol
		self.send_command(0x2c)
		self.send_data(0xa8)

		# DummyLine
		self.send_command(0x3a)
		self.send_data(0x1a)

		#Gatetime
		self.send_command(0x3b)
		self.send_data(0x08)

		#setRamDataEntryMode
		self.send_command(0x11)
		self.send_data(0x01)

		#_SetRamArea	
		self.send_command(0x44)
		self.send_data(0x00)
		self.send_data(0x0f)
		self.send_command(0x45)
		self.send_data(249)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)

		#_SetRamPointer
		self.send_command(0x4e)
		self.send_data(0x00)
		self.send_command(0x4f)
		self.send_data(249)
		self.send_data(0x00)

		#LUTDefault_full
		self.send_command(0x32)
		self.send_data(0x18)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x0F)
		self.send_data(0x01)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)

		#PowerOn
		self.send_command(0x22)
		self.send_data(0xc0)
		self.send_command(0x20)
		self.wait_until_idle()

		return 0

	def _spi_transfer(self, data):
		self.cs_pin.value(0)
		self.spi.write(data.to_bytes(1,'little'))
		self.cs_pin.value(1)

	def delay_ms(self, delaytime):
		utime.sleep_ms(delaytime)

	def send_command(self, command):
		self.dc_pin.value(0)
		self._spi_transfer(command)

	def send_data(self, data):
		self.dc_pin.value(1)
		self._spi_transfer(data)

	def wait_until_idle(self):
		while(self.busy_pin.value() == 1): # 0: idle, 1: busy
			self.delay_ms(100)

	def reset(self):
		self.reset_pin.value(0) # module reset
		self.delay_ms(200)
		self.reset_pin.value(1)
		self.delay_ms(200)

	def clear_frame(self, frame_buffer):
		for i in range(int(self.width * self.height / 8)):
			frame_buffer[i] = 0xFF

	def display_frame(self, frame_buffer):
		if (frame_buffer != None):
			#self.init()
			self.send_command(0x24)
			for i in range(0, EPD_HEIGHT * EPD_WIDTH / 8):
				self.send_data(frame_buffer[i])

			#UpdateFull
			self.send_command(0x22)
			self.send_data(0xc4)
			self.send_command(0x20)
			self.wait_until_idle()
			self.send_command(0xFF)

			#PowerOff
			self.send_command(0x22)
			self.send_data(0xc3)
			self.send_command(0x20)
			self.wait_until_idle()

	# after this, call epd.init() to awaken the module
	def sleep(self):
		self.send_command(VCOM_AND_DATA_INTERVAL_SETTING)
		self.send_data(0x17)
		self.send_command(VCM_DC_SETTING_REGISTER)		 #to solve Vcom drop
		self.send_data(0x00)
		self.send_command(POWER_SETTING)		 #power setting
		self.send_data(0x02)		#gate switch to external
		self.send_data(0x00)
		self.send_data(0x00)
		self.send_data(0x00)
		self.wait_until_idle()
		self.send_command(POWER_OFF)		 #power off

	def set_rotate(self, rotate):
		if (rotate == ROTATE_0):
			self.rotate = ROTATE_0
			self.width = EPD_WIDTH
			self.height = EPD_HEIGHT
		elif (rotate == ROTATE_90):
			self.rotate = ROTATE_90
			self.width = EPD_HEIGHT
			self.height = EPD_WIDTH
		elif (rotate == ROTATE_180):
			self.rotate = ROTATE_180
			self.width = EPD_WIDTH
			self.height = EPD_HEIGHT
		elif (rotate == ROTATE_270):
			self.rotate = ROTATE_270
			self.width = EPD_HEIGHT
			self.height = EPD_WIDTH

	def set_pixel(self, frame_buffer, x, y, colored):
		if (x < 0 or x >= self.width or y < 0 or y >= self.height):
			return
		if (self.rotate == ROTATE_0):
			self.set_absolute_pixel(frame_buffer, x, y, colored)
		elif (self.rotate == ROTATE_90):
			point_temp = x
			x = EPD_VISABLE_WIDTH - y
			y = point_temp
			self.set_absolute_pixel(frame_buffer, x, y, colored)
		elif (self.rotate == ROTATE_180):
			x = EPD_VISIBLE_WIDTH - x
			y = EPD_HEIGHT - y
			self.set_absolute_pixel(frame_buffer, x, y, colored)
		elif (self.rotate == ROTATE_270):
			point_temp = x
			x = y
			y = EPD_HEIGHT - point_temp
			self.set_absolute_pixel(frame_buffer, x, y, colored)

	def set_absolute_pixel(self, frame_buffer, x, y, colored):
		# To avoid display orientation effects
		# use EPD_WIDTH instead of self.width
		# use EPD_HEIGHT instead of self.height
		y = EPD_HEIGHT - y;
		if (x < 0 or x >= EPD_WIDTH or y < 0 or y >= EPD_HEIGHT):
			return
		if (colored):
			frame_buffer[int((x + y * EPD_WIDTH) / 8)] &= ~(0x80 >> (x % 8))
		else:
			frame_buffer[int((x + y * EPD_WIDTH) / 8)] |= 0x80 >> (x % 8)


	def draw_custom_font_char(self,frame_buffer,x,y,size,char,font,color):
		#offsets into glyph data array
		glyph_bitmap_ofset = 0
		glyph_width = 1
		glyph_height = 2
		glyph_xAdvance = 3
		glyph_xOffset = 4
		glyph_yOffset = 5

		#font.first_char
		#font.last_Char
		#font.y_advance
		
		char  = ord(char) - font.first_char
		bo    = font.Glyphs[char][glyph_bitmap_ofset]
		w     = font.Glyphs[char][glyph_width]
		h     = font.Glyphs[char][glyph_height]
		xo    = font.Glyphs[char][glyph_xOffset]
		yo    = font.Glyphs[char][glyph_yOffset]

		xx    = 0
		yy    = 0
		bits  = 0
		bit   = 0
		xo16  = 0
		yo16  = 0

		if size > 1:
			xo16 = int(xo)
			yo16 = int(yo)

		# copy glyph to screen
		yy = 0
		while (yy<h):
			xx = 0
			while (xx<w):
				#
				if (bit&0x07 == 0):
					bits = font.Bitmaps[bo]
					bo += 1
				if (bits&0x80 == 0x80):
					if size==1:
						self.set_pixel(frame_buffer,x+xo+xx,y+yo+yy,color)
					else:
						xp = x + (xo16 + xx) * size
						yp = y + (yo16 + yy) * size
						self.draw_filled_rectangle(frame_buffer,xp,yp,xp+size, yp+size, color)
				bit += 1
				bits = bits<<1
				xx += 1
			yy += 1
		pass
	
	# (fb, 0, 0, "OHS 2018", font24, gxgde0213b1.COLORED)
	def G_display_string_at(self,frame_buffer,x,y,string,font,size,color):
		#offsets into glyph data array
		glyph_bitmap_ofset = 0
		glyph_width = 1
		glyph_height = 2
		glyph_xAdvance = 3
		glyph_xOffset = 4
		glyph_yOffset = 5
		cursor_x = x
		cursor_y = y + (size * font.y_advance)
		
		for c in string:
			if c == '\n':
				cursor_x = 0
				cursor_y += size * font.y_advance
			elif c != '\r':
				char  = ord(c) - font.first_char
				w     = font.Glyphs[char][glyph_width]
				h     = font.Glyphs[char][glyph_height]
				if w >= 0 and h >= 0:
					xo    = font.Glyphs[char][glyph_xOffset]
					yo    = font.Glyphs[char][glyph_yOffset]
					xa    = font.Glyphs[char][glyph_xAdvance]
					self.draw_custom_font_char(frame_buffer,cursor_x,cursor_y,size,c,font,color)
					cursor_x += xa * size
	
		

	def draw_char_at(self, frame_buffer, x, y, char, font, colored):
		char_offset = (ord(char) - ord(' ')) * font.height * (int(font.width / 8) + (1 if font.width % 8 else 0))
		offset = 0

		for j in range(font.height):
			for i in range(font.width):
				if font.data[char_offset+offset] & (0x80 >> (i % 8)):
					self.set_pixel(frame_buffer, x + i, y + j, colored)
				if i % 8 == 7:
					offset += 1
			if font.width % 8 != 0: 
				offset += 1

	def display_string_at(self, frame_buffer, x, y, text, font, colored):
		refcolumn = x

		# Send the string character by character on EPD
		for index in range(len(text)):
			# Display one character on EPD
			self.draw_char_at(frame_buffer, refcolumn, y, text[index], font, colored)
			# Decrement the column position by 16
			refcolumn += font.width

	def draw_line(self, frame_buffer, x0, y0, x1, y1, colored):
		# Bresenham algorithm
		dx = abs(x1 - x0)
		sx = 1 if x0 < x1 else -1
		dy = -abs(y1 - y0)
		sy = 1 if y0 < y1 else -1
		err = dx + dy
		while((x0 != x1) and (y0 != y1)):
			self.set_pixel(frame_buffer, x0, y0 , colored)
			if (2 * err >= dy):
				err += dy
				x0 += sx
			if (2 * err <= dx):
				err += dx
				y0 += sy

	def draw_horizontal_line(self, frame_buffer, x, y, width, colored):
		for i in range(x, x + width):
			self.set_pixel(frame_buffer, i, y, colored)

	def draw_vertical_line(self, frame_buffer, x, y, height, colored):
		for i in range(y, y + height):
			self.set_pixel(frame_buffer, x, i, colored)

	def draw_rectangle(self, frame_buffer, x0, y0, x1, y1, colored):
		min_x = x0 if x1 > x0 else x1
		max_x = x1 if x1 > x0 else x0
		min_y = y0 if y1 > y0 else y1
		max_y = y1 if y1 > y0 else y0
		self.draw_horizontal_line(frame_buffer, min_x, min_y, max_x - min_x + 1, colored)
		self.draw_horizontal_line(frame_buffer, min_x, max_y, max_x - min_x + 1, colored)
		self.draw_vertical_line(frame_buffer, min_x, min_y, max_y - min_y + 1, colored)
		self.draw_vertical_line(frame_buffer, max_x, min_y, max_y - min_y + 1, colored)

	def draw_filled_rectangle(self, frame_buffer, x0, y0, x1, y1, colored):
		min_x = x0 if x1 > x0 else x1
		max_x = x1 if x1 > x0 else x0
		min_y = y0 if y1 > y0 else y1
		max_y = y1 if y1 > y0 else y0
		for i in range(min_x, max_x + 1):
			self.draw_vertical_line(frame_buffer, i, min_y, max_y - min_y + 1, colored)

	def draw_circle(self, frame_buffer, x, y, radius, colored):
		# Bresenham algorithm
		x_pos = -radius
		y_pos = 0
		err = 2 - 2 * radius
		if (x >= self.width or y >= self.height):
			return
		while True:
			self.set_pixel(frame_buffer, x - x_pos, y + y_pos, colored)
			self.set_pixel(frame_buffer, x + x_pos, y + y_pos, colored)
			self.set_pixel(frame_buffer, x + x_pos, y - y_pos, colored)
			self.set_pixel(frame_buffer, x - x_pos, y - y_pos, colored)
			e2 = err
			if (e2 <= y_pos):
				y_pos += 1
				err += y_pos * 2 + 1
				if(-x_pos == y_pos and e2 <= x_pos):
					e2 = 0
			if (e2 > x_pos):
				x_pos += 1
				err += x_pos * 2 + 1
			if x_pos > 0:
				break

	def draw_filled_circle(self, frame_buffer, x, y, radius, colored):
		# Bresenham algorithm
		x_pos = -radius
		y_pos = 0
		err = 2 - 2 * radius
		if (x >= self.width or y >= self.height):
			return
		while True:
			self.set_pixel(frame_buffer, x - x_pos, y + y_pos, colored)
			self.set_pixel(frame_buffer, x + x_pos, y + y_pos, colored)
			self.set_pixel(frame_buffer, x + x_pos, y - y_pos, colored)
			self.set_pixel(frame_buffer, x - x_pos, y - y_pos, colored)
			self.draw_horizontal_line(frame_buffer, x + x_pos, y + y_pos, 2 * (-x_pos) + 1, colored)
			self.draw_horizontal_line(frame_buffer, x + x_pos, y - y_pos, 2 * (-x_pos) + 1, colored)
			e2 = err
			if (e2 <= y_pos):
				y_pos += 1
				err += y_pos * 2 + 1
				if(-x_pos == y_pos and e2 <= x_pos):
					e2 = 0
			if (e2 > x_pos):
				x_pos  += 1
				err += x_pos * 2 + 1
			if x_pos > 0:
				break

	def draw_bmp(self, frame_buffer, image_path, colored):
		self.draw_bmp_at(frame_buffer, 0, 0, image_path, colored)

	def draw_bmp_at(self, frame_buffer, x, y, image_path, colored):
		if x >= self.width or y >= self.height:
			return

		try:
			with open(image_path, 'rb') as bmp_file:
				header = BitmapHeader(bmp_file.read(BitmapHeader.SIZE_IN_BYTES))
				header_info = BitmapHeaderInfo(bmp_file.read(BitmapHeaderInfo.SIZE_IN_BYTES))
				data_end = header.file_size - 2

				if header_info.width > self.width:
					widthClipped = self.width
				elif x < 0:
					widthClipped = header_info.width + x
				else:
					widthClipped = header_info.width

				if header_info.height > self.height:
					heightClipped = self.height
				elif y < 0:
					heightClipped = header_info.height + y
				else:
					heightClipped = header_info.height

				heightClipped = max(0, min(self.height-y, heightClipped))
				y_offset = max(0, -y)

				if heightClipped <= 0 or widthClipped <= 0:
					return

				width_in_bytes = int(self.width/8)
				if header_info.width_in_bytes > width_in_bytes:
					rowBytesClipped = width_in_bytes
				else:
					rowBytesClipped = header_info.width_in_bytes

				for row in range(y_offset, heightClipped):
					absolute_row = row + y
					# seek to beginning of line
					bmp_file.seek(data_end - (row + 1) * header_info.line_width)

					line = bytearray(bmp_file.read(rowBytesClipped))
					if header_info.last_byte_padding > 0:
						mask = 0xFF<<header_info.last_byte_padding & 0xFF
						line[-1] &= mask

					for byte_index in range(len(line)):
						byte = line[byte_index]
						for i in range(8):
							if byte & (0x80 >> i):
								self.set_pixel(frame_buffer, byte_index*8 + i + x, absolute_row, colored)

		except OSError as e:
			print('error: {}'.format(e))

### END OF FILE ###
