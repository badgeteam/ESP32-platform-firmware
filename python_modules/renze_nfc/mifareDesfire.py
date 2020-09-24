import pn532

# Status and error codes
DF_RETURN_OK                      = 0x00
DF_RETURN_NO_CHANNGES             = 0x0C
DF_RETURN_OUT_OF_EEPROM_ERROR     = 0x0E
DF_RETURN_ILLEGAL_COMMAND_CODE    = 0x1C
DF_RETURN_INTEGRITY_ERROR         = 0x1E
DF_RETURN_NO_SUCH_KEY             = 0x40
DF_RETURN_LENGTH_ERROR            = 0x7E
DF_RETURN_PERMISSION_DENIED       = 0x9D
DF_RETURN_PARAMETER_ERROR         = 0x9E
DF_RETURN_APPLICATION_NOT_FOUND   = 0xA0
DF_RETURN_APPL_INTEGRITY_ERROR    = 0xA1
DF_RETURN_AUTHENTICATION_ERROR    = 0xAE
DF_RETURN_ADDITIONAL_FRAME        = 0xAF
DF_RETURN_BOUNDARY_ERROR          = 0xBE
DF_RETURN_PICC_INTEGRITY_ERROR    = 0xC1
DF_RETURN_COMMAND_ABORTED         = 0xCA
DF_RETURN_PICC_DISABLED_ERROR     = 0xCD
DF_RETURN_COUNT_ERROR             = 0xCE
DF_RETURN_DUPLICATE_ERROR         = 0xDE
DF_RETURN_EEPROM_ERROR            = 0xEE
DF_RETURN_FILE_NOT_FOUND          = 0xF0
DF_RETURN_FILE_INTEGRITY_ERROR    = 0xF1

# Communication commands
DF_CMD_ADDITIONAL_FRAME           = 0xAF

# Security related commands 
DF_CMD_AUTHENTICATE_LEGACY        = 0x0A
DF_CMD_CHANGE_KEYSETTINGS         = 0x54
DF_CMD_GET_KEYSETTINGS            = 0x45
DF_CMD_CHANGE_KEY                 = 0xC4
DF_CMD_GET_KEYVERSION             = 0x64

# PICC level commands
DF_CMD_CREATE_APPLICATION         = 0xCA
DF_CMD_DELETE_APPLICATION         = 0xDA
DF_CMD_GET_APPLICATION_IDS        = 0x6A
DF_CMD_SELECT_APPLICATION         = 0x5A
DF_CMD_FORMAT_PICC                = 0xFC
DF_CMD_GET_VERSION                = 0x60

# Application level commands
DF_CMD_GET_FILE_IDS               = 0x6F
DF_CMD_GET_FILE_SETTINGS          = 0xF5
DF_CMD_CHANGE_FILE_SETTINGS       = 0x5F
DF_CMD_CREATE_STD_DATA_FILE       = 0xCD
DF_CMD_CREATE_BACKUP_DATA_FILE    = 0xCB
DF_CMD_CREATE_VALUE_FILE          = 0xCC
DF_CMD_CREATE_LINEAR_RECORD_FILE  = 0xC1
DF_CMD_CREATE_CYCLIC_RECORD_FILE  = 0xC0
DF_CMD_DELETE_FILE                = 0xDF

# Data manipulation commands
DF_CMD_READ_DATA                  = 0xBD
DF_CMD_WRITE_DATA                 = 0x3D
DF_CMD_GET_VALUE                  = 0x6C
DF_CMD_CREDIT                     = 0x0C
DF_CMD_DEBIT                      = 0xDC
DF_CMD_LIMITED_CREDIT             = 0x1C
DF_CMD_WRITE_RECORD               = 0x3B
DF_CMD_READ_RECORDS               = 0xBB
DF_CMD_CLEAR_RECORD_FILE          = 0xEB
DF_CMD_COMMIT_TRANSACTION         = 0xC7
DF_CMD_ABORT_TRANSACTION          = 0xA7

# Desfire EV1 commands
DFEV1_CMD_AUTHENTICATE_ISO        = 0x1A
DFEV1_CMD_AUTHENTICATE_AES        = 0xAA
DFEV1_CMD_FREE_MEM                = 0x6E
DFEV1_CMD_GET_DF_NAMES            = 0x6D
DFEV1_CMD_GET_CARD_UID            = 0x51
DFEV1_CMD_GET_ISO_FILE_IDS        = 0x61
DFEV1_CMD_SET_CONFIGURATION       = 0x5C

# ISO7816 instructions
ISO7816_CMD_EXTERNAL_AUTHENTICATE = 0x82
ISO7816_CMD_INTERNAL_AUTHENTICATE = 0x88
ISO7816_CMD_APPEND_RECORD         = 0xE2
ISO7816_CMD_GET_CHALLENGE         = 0x84
ISO7816_CMD_READ_RECORDS          = 0xB2
ISO7816_CMD_SELECT_FILE           = 0xA4
ISO7816_CMD_READ_BINARY           = 0xB0
ISO7816_CMD_UPDATE_BINARY         = 0xD6

# Key types
DF_KEY_2K3DES                     = 0x00 # DFEV1_CMD_AUTHENTICATE_ISO + DF_CMD_AUTHENTICATE_LEGACY
DF_KEY_3K3DES                     = 0x40 # DFEV1_CMD_AUTHENTICATE_ISO
DF_KEY_AES                        = 0x80 # DFEV1_CMD_AUTHENTICATE_AES
DF_KEY_INVALID                    = 0xFF

class Key:
	def __init__(self, size, blocksize, version, keyType):
		self.size = size
		self.blocksize = blocksize
		self.version = version
		self.keyType = keyType



def isMifareDesfire(atqa, sak):
	return (atqa == 0x4403) and (sak == 0x20)

def getCardInfo():
	result = [
		pn532.transaction([DF_CMD_GET_VERSION], 64),
		pn532.transaction([DF_CMD_ADDITIONAL_FRAME], 64),
		pn532.transaction([DF_CMD_ADDITIONAL_FRAME], 64)
		]
	hwVendorId      = result[0][1]
	hwType          = result[0][2]
	hwSubType       = result[0][3]
	hwMajorVersion  = result[0][4]
	hwMinorVersion  = result[0][5]
	hwStorageSize   = result[0][6]
	hwStorageBytes  = 2**(hwStorageSize>>1)
	hwStorageBytesM = bool(hwStorageSize&0x01)
	hwProtocol      = result[0][7]

	swVendorId      = result[1][1]
	swType          = result[1][2]
	swSubType       = result[1][3]
	swMajorVersion  = result[1][4]
	swMinorVersion  = result[1][5]
	swStorageSize   = result[1][6]
	swStorageBytes  = 2**(swStorageSize>>1)
	swStorageBytesM = bool(swStorageSize&0x01)
	swProtocol      = result[1][7]

	serialNumber    = result[2][1:7]
	productionBatch = result[2][8:12]
	productionWeek  = result[2][13]
	productionYear  = result[2][14]
	return (hwVendorId, hwType, hwSubType, hwMajorVersion, hwMinorVersion, hwStorageBytes, hwStorageBytesM, hwProtocol, swVendorId, swType, swSubType, swMajorVersion, swMinorVersion, swStorageBytes, swStorageBytesM, swProtocol, serialNumber, productionBatch, productionWeek, productionYear)

def printCardInfo():
	info = getCardInfo()
	print("Card information\n===========================")
	print("")
	print("Hardware vendor ID    ", info[0])
	print("Hardware type         ", info[1])
	print("Hardware version      ", str(info[2])+"."+str(info[3]))
	print("Hardware storage size ", str(info[4])+("+" if info[5] else "")+" bytes")
	print("Hardware protocol     ", info[6])
	print("")
	print("Software vendor ID    ", info[7])
	print("Software type         ", info[8])
	print("Software version      ", str(info[9])+"."+str(info[10]))
	print("Hardware storage size ", str(info[11])+("+" if info[12] else "")+" bytes")
	print("Software protocol     ", info[13])
	print("")
	print("Serial number         ", (binascii.hexlify(bytes(info[14]))).decode("ascii"))
	print("Production batch      ", (binascii.hexlify(bytes(info[15]))).decode("ascii"))
	print("Production date       ", "week "+hex(info[16])[2:]+" of 20"+hex(info[17])[2:])
	

def test():
	pn532.SAM_configuration()
	uid, atqa, sak = read_passive_target()
	if uid != None:
		print("Found card with UID {}".format(pn532.uidToStr(uid)))
		if not isMifareDesfire(atqa, sak):
			print(" -> Card is not a Mifare Desfire card!")
			return
