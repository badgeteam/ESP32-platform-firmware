import machine, time, binascii#, i2c

PN532_PREAMBLE                      = 0x00
PN532_STARTCODE1                    = 0x00
PN532_STARTCODE2                    = 0xFF
PN532_POSTAMBLE                     = 0x00

PN532_HOSTTOPN532                   = 0xD4
PN532_PN532TOHOST                   = 0xD5

# PN532 Commands
PN532_COMMAND_DIAGNOSE              = 0x00
PN532_COMMAND_GETFIRMWAREVERSION    = 0x02
PN532_COMMAND_GETGENERALSTATUS      = 0x04
PN532_COMMAND_READREGISTER          = 0x06
PN532_COMMAND_WRITEREGISTER         = 0x08
PN532_COMMAND_READGPIO              = 0x0C
PN532_COMMAND_WRITEGPIO             = 0x0E
PN532_COMMAND_SETSERIALBAUDRATE     = 0x10
PN532_COMMAND_SETPARAMETERS         = 0x12
PN532_COMMAND_SAMCONFIGURATION      = 0x14
PN532_COMMAND_POWERDOWN             = 0x16
PN532_COMMAND_RFCONFIGURATION       = 0x32
PN532_COMMAND_RFREGULATIONTEST      = 0x58
PN532_COMMAND_INJUMPFORDEP          = 0x56
PN532_COMMAND_INJUMPFORPSL          = 0x46
PN532_COMMAND_INLISTPASSIVETARGET   = 0x4A
PN532_COMMAND_INATR                 = 0x50
PN532_COMMAND_INPSL                 = 0x4E
PN532_COMMAND_INDATAEXCHANGE        = 0x40
PN532_COMMAND_INCOMMUNICATETHRU     = 0x42
PN532_COMMAND_INDESELECT            = 0x44
PN532_COMMAND_INRELEASE             = 0x52
PN532_COMMAND_INSELECT              = 0x54
PN532_COMMAND_INAUTOPOLL            = 0x60
PN532_COMMAND_TGINITASTARGET        = 0x8C
PN532_COMMAND_TGSETGENERALBYTES     = 0x92
PN532_COMMAND_TGGETDATA             = 0x86
PN532_COMMAND_TGSETDATA             = 0x8E
PN532_COMMAND_TGSETMETADATA         = 0x94
PN532_COMMAND_TGGETINITIATORCOMMAND = 0x88
PN532_COMMAND_TGRESPONSETOINITIATOR = 0x90
PN532_COMMAND_TGGETTARGETSTATUS     = 0x8A

PN532_RESPONSE_INDATAEXCHANGE       = 0x41
PN532_RESPONSE_INLISTPASSIVETARGET  = 0x4B

PN532_WAKEUP                        = 0x55

PN532_SPI_STATREAD                  = 0x02
PN532_SPI_DATAWRITE                 = 0x01
PN532_SPI_DATAREAD                  = 0x03
PN532_SPI_READY                     = 0x01

PN532_MIFARE_ISO14443A              = 0x00

# Mifare Commands
MIFARE_CMD_AUTH_A                   = 0x60
MIFARE_CMD_AUTH_B                   = 0x61
MIFARE_CMD_READ                     = 0x30
MIFARE_CMD_WRITE                    = 0xA0
MIFARE_CMD_TRANSFER                 = 0xB0
MIFARE_CMD_DECREMENT                = 0xC0
MIFARE_CMD_INCREMENT                = 0xC1
MIFARE_CMD_STORE                    = 0xC2
MIFARE_ULTRALIGHT_CMD_WRITE         = 0xA2

PN532_GPIO_VALIDATIONBIT            = 0x80
PN532_GPIO_P30                      = 0
PN532_GPIO_P31                      = 1
PN532_GPIO_P32                      = 2
PN532_GPIO_P33                      = 3
PN532_GPIO_P34                      = 4
PN532_GPIO_P35                      = 5

PN532_ACK                           = bytearray([0x01, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00])
PN532_FRAME_START                   = bytearray([0x01, 0x00, 0x00, 0xFF])

PN532_ADDR                          = 0x24

def reduce(function, iterable, initializer=None):
    it = iter(iterable)
    if initializer is None:
        value = next(it)
    else:
        value = initializer
    for element in it:
        value = function(value, element)
    return value

def uint8_add(a, b):
	"""Add add two values as unsigned 8-bit values."""
	return ((a & 0xFF) + (b & 0xFF)) & 0xFF

def write_frame(data):
	length = len(data)
	frame = []
	frame.append(PN532_SPI_DATAWRITE)
	frame.append(PN532_PREAMBLE)
	frame.append(PN532_STARTCODE1)
	frame.append(PN532_STARTCODE2)
	frame.append(length & 0xFF)
	frame.append(uint8_add(~length, 1))
	frame.extend(data)
	checksum = reduce(uint8_add, data, 0xFF)
	frame.append(~checksum & 0xFF)
	frame.append(PN532_POSTAMBLE)
	i2c.writeto(PN532_ADDR, bytearray(frame))

def read_frame(amount):
	response = i2c.readfrom(PN532_ADDR, amount+8)
	if response[0] != 0x01:
		raise RuntimeError('Response frame does not start with 0x01!')
	offset = 1
	while response[offset] == 0x00:
		offset += 1
		if offset >= len(response):
			raise RuntimeError("Response frame preamble does not contain 0x00FF! (0x00)")
	if response[offset] != 0xFF:
		raise RuntimeError('Response frame preamble does not contain 0x00FF! (0xFF) == {}'.format(response[offset]))
	offset += 1
	if offset >= len(response):
		raise RuntimeError('Response contains no data!')
	frame_len = response[offset]
	if (frame_len + response[offset+1]) & 0xFF != 0:
		raise RuntimeError('Response length checksum did not match length!')
	checksum = reduce(uint8_add, response[offset+2:offset+2+frame_len+1], 0)
	if checksum != 0:
		#print("CHKSUM INVALID", response, checksum)
		raise RuntimeError('Response checksum did not match expected value!')
	return response[offset+2:offset+2+frame_len]

def wait_ready(timeout_sec=1):
	start = time.time()
	while True:
		response = i2c.readfrom(PN532_ADDR, 1)
		if response[0] == PN532_SPI_READY:
			return True
		if time.time() - start >= timeout_sec:
			return False
		time.sleep(0.05)
		
def call_function(command, response_length=0, params=[], timeout_sec=1):
	data = []
	data.append(PN532_HOSTTOPN532)
	data.append(command & 0xFF)
	data.extend(params)
	write_frame(data)
	if not wait_ready(timeout_sec):
		return None
	response = i2c.readfrom(PN532_ADDR, len(PN532_ACK))
	if response != PN532_ACK:
		raise RuntimeError('Did not receive expected ACK from PN532!')
	if not wait_ready(timeout_sec):
		return None
	response = read_frame(response_length+2)
	if not (response[0] == PN532_PN532TOHOST and response[1] == (command+1)):
		print(response)
		print(hex(response[0]), hex(PN532_PN532TOHOST))
		print(hex(response[1]), hex(command+1))
		raise RuntimeError('Received unexpected command response!')
	#print("Response length", len(response[2:]))
	return response[2:]

def get_firmware_version():
	response = call_function(PN532_COMMAND_GETFIRMWAREVERSION, 4)
	if response is None:
		raise RuntimeError('Failed to detect the PN532!')
	return (response[0], response[1], response[2], response[3])

def read_passive_target(card_baud=PN532_MIFARE_ISO14443A, timeout_sec=1):
	response = call_function(PN532_COMMAND_INLISTPASSIVETARGET, params=[0x01, card_baud], response_length=20)
	if response is None:
		return None
	if response[0] != 0x01:
		raise RuntimeError('More than one card detected!')
	if response[5] > 7:
		raise RuntimeError('Found card with unexpectedly long UID!')
	#print(binascii.hexlify(response))
	return response[6:6+response[5]], (response[3]<<8)+response[2], response[4]

def SAM_configuration():
	call_function(PN532_COMMAND_SAMCONFIGURATION, params=[0x01, 0x05, 0x01])

def mifare_classic_authenticate_block(uid, block_number, key_number, key):
	uidlen = len(uid)
	keylen = len(key)
	params = [0x01, key_number & 0xFF, block_number & 0xFF]
	params.extend(key)
	params.extend(uid)
	response = call_function(PN532_COMMAND_INDATAEXCHANGE, 1, params)
	return response[0] == 0x00

def mifare_classic_read_block(block_number):
	response = call_function(PN532_COMMAND_INDATAEXCHANGE, 17, [0x01, MIFARE_CMD_READ, block_number & 0xFF])
	if response[0] != 0x00:
		return None
	return response[1:]

def mifare_classic_write_block(block_number, data):
	assert data is not None and len(data) == 16, 'Data must be an array of 16 bytes!'
	params = [0x01, MIFARE_CMD_WRITE, block_number & 0xFF]
	params.extend(data)
	response = self.call_function(PN532_COMMAND_INDATAEXCHANGE, 1, params)
	return response[0] == 0x00

def uidToStr(uid):
	uidStr = ""
	for i in range(len(uid)):
		item = hex(uid[i])[2:]
		if len(item) < 2:
			item = "0"+item
		uidStr += item
		if i < len(uid)-1:
			uidStr += ":"
	return uidStr

#i2c = machine.I2C(sda=machine.Pin(i2c.GPIO_SDA), scl=machine.Pin(i2c.GPIO_CLK), freq=i2c.SPEED)
i2c = machine.I2C(sda=machine.Pin(22), scl=machine.Pin(21), freq=40000)

print("Device info:", get_firmware_version())

def getCardType(atqa, sak):
	if atqa == 0x0400:
		if sak == 0x09:
			return "MIFARE Mini"
		if sak == 0x08 or sak == 0x88:
			return "MIFARE Classic 1k"	
		if sak == 0x00:
			return "MIFARE Ultralight"
	if atqa == 0x0200:
		if sak == 0x18:
			return "MIFARE Classic 4k"
	if atqa == 0x4403:
		if sak == 0x20:
			return "MIFARE DESFire"
	if atqa == 0x0200:
		if sak == 0x38:
			return "MIFARE Classic 4k - emulated (6212)"
	if atqa == 0x0800:
		if sak == 0x38:
			return "MIFARE Classic 4k - emulated (6131)"
	return "Unknown"

def test():
	SAM_configuration()
	uid, atqa, sak = read_passive_target()
	if uid != None:
		print("Found card with UID {}".format(uidToStr(uid)))
		cardType = getCardType(atqa, sak)
		print("Type:",cardType)
		if cardType == "MIFARE Classic 1k":
			if not mifare_classic_authenticate_block(uid, 4, MIFARE_CMD_AUTH_B, [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]):
				print("Failed to authenticate block 4!")
				return
			data = mifare_classic_read_block(4)
			if data is None:
				print('Failed to read block 4!')
				return
			print('Read block 4: 0x{0}'.format(binascii.hexlify(data[:4])))

def transfer(out):
	res = call_function(PN532_COMMAND_INDATAEXCHANGE, 256, out)
	if res[0] != 0x00:
		raise RuntimeError("Communication error")
	return res[1:]

