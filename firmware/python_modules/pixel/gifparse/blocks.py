import struct
from .core import Block, SubBlock

class Header(Block):
    pass

class LogicalScreenDescriptor(Block):
    def parse(self):
        print('Reading LSD')
        packed_field = '{0:08b}'.format(int(str(self.raw[4]).encode("hex"), 16))
        self.global_color_flag = int(packed_field[0], 2)
        self.color_resolution = int(packed_field[1:4], 2)
        self.color_table_length = pow(2, int(packed_field[4:], 2) + 1)

class GlobalColorTable(Block):
    pass

class Trailer(Block):
    pass

class Extension(Block):
    pass

class ApplicationExtension(Extension):
    @classmethod
    def extract(cls, io):
        print('Reading AE')
        ext_bytes = "\x21\xff"
        block_size_bytes = io.read(1)
        block_size = int(block_size_bytes.encode("hex"), 16)
        ext_info = io.read(block_size)
        ext_bytes += block_size_bytes + ext_info
        while True:
            subblock = SubBlock.extract(io)
            ext_bytes += subblock.raw
            if subblock.raw[0] == "\x00": break
        return cls(ext_bytes)

class GraphicsControlExtension(Extension):
    prefix = "\x21\xf9"

    @classmethod
    def extract(cls, io):
        print('Reading GCE')
        ext = cls(0)
        ext.block_size_bytes = io.read(1)
        block_size = int(ext.block_size_bytes.encode("hex"), 16)
        ext.packed_byte = io.read(1)
        unpacked_bits = '{0:08b}'.format(int(ext.packed_byte.encode("hex"), 16))
        ext.delay_bytes = io.read(2)
        ext.delay = struct.unpack('H', ext.delay_bytes)[0]
        ext.transparent_color_index = io.read(1)
        ext.terminator = io.read(1)
        ext.raw = ext.reconstruct_bytes()
        return ext

    def set_delay(self, centiseconds):
        self.delay = centiseconds
        self.delay_bytes = struct.pack("H", centiseconds)
        self.raw = self.reconstruct_bytes()
        return self

    def reconstruct_bytes(self):
        return "".join([
            self.prefix,
            self.block_size_bytes,
            self.packed_byte,
            self.delay_bytes,
            self.transparent_color_index,
            self.terminator
        ])

class CommentExtension(Extension):
    prefix = "\x21\xfe"

    @classmethod
    def extract(cls, io):
        print('Reading CE')
        ext_bytes = cls.prefix
        while True:
            subblock = SubBlock.extract(io)
            ext_bytes += subblock.raw
            if subblock.raw[0] == "\x00": break
        return ext

    @classmethod
    def encode(cls, comment_text):
        ext_bytes = cls.prefix
        sub_blocks = SubBlock.encode(comment_text)
        ext_bytes += "".join(sb.raw for sb in sub_blocks)
        return cls(ext_bytes)

class ImageBlock(Block):
    @classmethod
    def extract(cls, io):
        print('Reading image block')
        img_bytes = "\x2c"
        left = io.read(2)
        top = io.read(2)
        width = io.read(2)
        height = io.read(2)
        packed_byte = io.read(1)
        unpacked_bits = '{0:08b}'.format(int(packed_byte.encode("hex"), 16))
        local_color_table_len = int(unpacked_bits[0]) * pow(2, int(unpacked_bits[4:], 2) + 1)
        local_color_table = io.read(3*local_color_table_len)
        lzw_min = io.read(1)
        img_bytes += left + top + width + height + packed_byte + local_color_table + lzw_min
        while True:
            subblock = SubBlock.extract(io)
            img_bytes += subblock.raw
            if subblock.raw[0] == "\x00": break
        img = cls(img_bytes)
        return img
