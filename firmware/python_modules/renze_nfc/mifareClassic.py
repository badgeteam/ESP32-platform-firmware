import pn532

# Card specific NFC functions for Mifare Classic and Ultralight cards

# Commands
MIFARE_CMD_AUTH_A                   = 0x60
MIFARE_CMD_AUTH_B                   = 0x61
MIFARE_CMD_READ                     = 0x30
MIFARE_CMD_WRITE                    = 0xA0
MIFARE_CMD_TRANSFER                 = 0xB0
MIFARE_CMD_DECREMENT                = 0xC0
MIFARE_CMD_INCREMENT                = 0xC1
MIFARE_CMD_STORE                    = 0xC2
MIFARE_ULTRALIGHT_CMD_WRITE         = 0xA2

def mifare_classic_authenticate_block(uid, block_number, key_number, key):
	uidlen = len(uid)
	keylen = len(key)
	params = [0x01, key_number & 0xFF, block_number & 0xFF]
	params.extend(key)
	params.extend(uid)
	response = pn532.call_function(pn532.PN532_COMMAND_INDATAEXCHANGE, 1, params)
	return response[0] == 0x00

def mifare_classic_read_block(block_number):
	response = pn532.call_function(pn532.PN532_COMMAND_INDATAEXCHANGE, 17, [0x01, MIFARE_CMD_READ, block_number & 0xFF])
	if response[0] != 0x00:
		return None
	return response[1:]

def mifare_classic_write_block(block_number, data):
	assert data is not None and len(data) == 16, 'Data must be an array of 16 bytes!'
	params = [0x01, MIFARE_CMD_WRITE, block_number & 0xFF]
	params.extend(data)
	response = pn532.call_function(pn532.PN532_COMMAND_INDATAEXCHANGE, 1, params)
	return response[0] == 0x00

def isMifareClassic1k(atqa, sak):
	return (atqa == 0x0400) and (sak == 0x08 or sak == 0x88)

def isMifareClassic4k(atqa, sak):
	return ((atqa == 0x0200) and (sak == 0x18 or sak == 0x38)) or (atqa == 0x0800 and sak == 0x38)

def isMifareUltralight(atqa, sak):
	return (atqa == 0x0400) and (sak == 0x00)

def isMifareMini(atqa, sak):
	return (atqa == 0x0400) and (sak == 0x09)


def getCardType(atqa, sak):
	if atqa == 0x0400:
		if sak == 0x09:
			return "MIFARE Mini"
		if sak == 0x00:
			return "MIFARE Ultralight"
	return "Unknown"

def test():
	pn532.SAM_configuration()
	uid, atqa, sak = read_passive_target()
	if uid != None:
		print("Found card with UID {}".format(pn532.uidToStr(uid)))
		if not (isMifareClassic1k(atqa, sak) or isMifareClassic4k(atqa, sak)):
			print(" -> Card is not a Mifare Classic card!")
			return
		if not mifare_classic_authenticate_block(uid, 4, MIFARE_CMD_AUTH_B, [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]):
			print("Failed to authenticate block 4!")
			return
		data = mifare_classic_read_block(4)
		if data is None:
			print('Failed to read block 4!')
			return
		print('Read block 4: 0x{0}'.format(binascii.hexlify(data[:4])))
