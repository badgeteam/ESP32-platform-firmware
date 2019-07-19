import struct
from uio import BytesIO

SEEK_BEG = 0
SEEK_CUR = 1
SEEK_END = 2

TERMINATOR = "\x00"

class Block(object):
    def __init__(self, raw_bytes):
        self.raw = raw_bytes
        self.parse()
    def parse(self):
        pass

class SubBlock(object):
    @classmethod
    def extract(cls, io):
        subblock_size_bytes = io.read(1)
        subblock_size = int(subblock_size_bytes.encode("hex"), 16)
        subblock_data = io.read(subblock_size)
        subblock = cls(subblock_size_bytes + subblock_data)
        subblock.data = subblock_data
        subblock.size = subblock_size
        return subblock

    @classmethod
    def encode(cls, data):
        sub_blocks = []
        stream = BytesIO(data)
        while True:
            next_bytes = stream.read(255)
            byte_len = struct.pack("B", len(next_bytes))
            sub_blocks.append(cls(byte_len + next_bytes))
            if not len(next_bytes): break
        return sub_blocks

    def __init__(self, raw_bytes):
        self.raw = raw_bytes
