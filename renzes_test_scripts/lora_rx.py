import lora, time

lora.set_frequency(int(868e6))
lora.enable_crc()
lora.set_spreading_factor(8)
lora.set_tx_power(27)

lora.set_mode_receive()
while 1:
	while (lora.received()):
		try:
			packet = lora.receive_packet()
			print("Received packet:", packet)
		except:
			print("Failed.")
		lora.set_mode_receive()
	time.sleep(0.01)
