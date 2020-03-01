import machine, time, lora

lora.set_frequency(int(868e6))
lora.enable_crc()
lora.set_spreading_factor(8)
lora.set_tx_power(27)

gps_uart = machine.UART(2, rx=12, tx=15, baudrate=9600, bits=8, parity=None, stop=1, timeout=1500, buffer_size=1024, lineend='\r\n')

gps = machine.GPS(gps_uart)
gps.startservice()

while 1:
	(time_tuple, latitude, longitude, altitude, n_satelites, quality, speed_kmh, course, dop) = gps.getdata()
	
	now = ""
	for i in time_tuple:
		now += str(i)+","
	
	message = now+str(latitude)+","+str(longitude)+","+str(altitude)+","+str(n_satelites)+","+str(quality)+","+str(speed_kmh)+","+str(course)+","+str(dop)
	print(message)
	print(len(message))
	lora.send_packet(bytes(message.encode("ascii")))
	time.sleep(2)
