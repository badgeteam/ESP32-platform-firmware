# DS18x20 temperature sensor driver for MicroPython.
# MIT license; Copyright (c) 2016 Damien P. George, Copyright (c) 2020 Badge.team

from micropython import const
import struct

_CONVERT = const(0x44)
_RD_SCRATCH = const(0xbe)
_WR_SCRATCH = const(0x4e)
_TEMP_CONVERT_CELCIUS = const(0x10)
_DATA_STRUCTURE = '<hBBBBBB'
_DATA_CONFIG_RESOLUTION_OFFSET = const(5)

class DS18X20:
    RESOLUTION_9BIT = 0
    RESOLUTION_10BIT = 1
    RESOLUTION_11BIT = 2
    RESOLUTION_12BIT = 3

    def __init__(self, onewire):
        self.ow = onewire
        self.buf = bytearray(9)

    def scan(self):
        return [rom for rom in self.ow.scan() if rom[0] == 0x10 or rom[0] == 0x28]

    def convert_temp(self):
        self.ow.reset(True)
        self.ow.writebyte(self.ow.SKIP_ROM)
        self.ow.writebyte(_CONVERT)

    def read_scratch(self, rom):
        self.ow.reset(True)
        self.ow.select_rom(rom)
        self.ow.writebyte(_RD_SCRATCH)
        self.ow.readinto(self.buf)
        if self.ow.crc8(self.buf):
            raise Exception('CRC error')
        return self.buf

    def write_scratch(self, rom, buf):
        self.ow.reset(True)
        self.ow.select_rom(rom)
        self.ow.writebyte(_WR_SCRATCH)
        self.ow.write(buf)

    def parse_scratch(self, data):
        [temperature, th, tl, config, res1, res2, res3] = struct.unpack(_DATA_STRUCTURE, data)
        return {
            'temperature': temperature,
            'th': th,
            'tl': tl,
            'config': config,
            'res1': res1,
            'res2': res2,
            'res3': res3
        }

    def assemble_scratch(self, data):
        return struct.pack(
            _DATA_STRUCTURE,
            data['temperature'],
            data['th'],
            data['tl'],
            data['config'],
            data['res1'],
            data['res2'],
            data['res3']
        )

    def set_resolution(self, rom, resolution):
        if (resolution < self.RESOLUTION_9BIT or resolution > self.RESOLUTION_12BIT):
            raise ValueError("Resolution is not valid")

        buf = self.read_scratch(rom)
        data = self.parse_scratch(buf)
        # The other bits don't care about being written to (r/o)
        data['config'] = resolution << _DATA_CONFIG_RESOLUTION_OFFSET
        buf = self.assemble_scratch(data)
        self.write_scratch(rom, buf)

    def read_temp(self, rom):
        buf = self.read_scratch(rom)
        data = self.parse_scratch(buf)

        resolution = (data['config'] >> _DATA_CONFIG_RESOLUTION_OFFSET) & 0x03

        # In the lower resolution modes these bits are not used.
        # We null them to prevent data inaccuracy

        if (resolution == self.RESOLUTION_9BIT):
            data['temperature'] &= ~0x07
        elif (resolution == self.RESOLUTION_10BIT):
            data['temperature'] &= ~0x03
        elif (resolution == self.RESOLUTION_11BIT):
            data['temperature'] &= ~0x01

        data['temperature'] /= _TEMP_CONVERT_CELCIUS

        return data['temperature']
