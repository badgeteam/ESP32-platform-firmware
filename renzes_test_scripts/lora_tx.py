import lora, time

lora.set_frequency(int(868e6))
lora.enable_crc()
lora.set_spreading_factor(8)
lora.set_tx_power(27)

while 1:
	lora.send_packet(bytes("Dit is een test!".encode("ascii")))
	time.sleep(0.5)
