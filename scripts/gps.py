import machine, time

gps_uart = machine.UART(2, rx=12, tx=15, baudrate=9600, bits=8, parity=None, stop=1, timeout=1500, buffer_size=1024, lineend='\r\n')

gps = machine.GPS(gps_uart)
gps.startservice()

while 1:
	print(gps.getdata())
	time.sleep(0.5)
