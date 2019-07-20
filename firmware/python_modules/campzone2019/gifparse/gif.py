import sys
from uio import BytesIO
from .blocks import Header
from .blocks import LogicalScreenDescriptor
from .blocks import GlobalColorTable
from .blocks import ApplicationExtension
from .blocks import GraphicsControlExtension
from .blocks import CommentExtension
from .blocks import ImageBlock
from .blocks import Trailer

SEEK_BEG = 0
SEEK_CUR = 1
SEEK_END = 2

class GIF(object):
    def __init__(self, raw):
        is_buffered = False
        # is_buffered = isinstance(raw, IOBase) or isinstance(raw, file)
        self.io = raw if is_buffered else BytesIO(raw)
        self.header = ""
        self.screen_desc = ""
        self.global_color_table = ""
        self.application_extensions = []
        self.comment_extensions = []
        self.frames = []
        self.trailer = ""
        self.parse()

    def parse(self):
        # GIF Header
        self.header = Header(self.io.read(6))
        if self.header.raw != b'GIF89a' and self.header.raw != b'GIF87a':
            sys.stderr.write("WARNING: File/bytes don't have GIF89a header. Parsing halted.\n")
            return False

        # Logical Screen Descriptor
        self.screen_desc = LogicalScreenDescriptor(self.io.read(7))
        if self.screen_desc.global_color_flag:
            gct_bytecount = self.screen_desc.color_table_length * 3
            gct_bytes = self.io.read(gct_bytecount)
            self.global_color_table = GlobalColorTable(gct_bytes)

        # Extension and Image Blocks
        last_gce = None
        while True:
            next_two_bytes = self.io.read(2)
            if next_two_bytes == "\x21\xff":
                ext = ApplicationExtension.extract(self.io)
                self.application_extensions.append(ext)
            elif next_two_bytes == "\x21\xfe":
                ext = CommentExtension.extract(self.io)
                self.comment_extensions.append(ext)
            elif next_two_bytes == "\x21\xf9":
                gce = GraphicsControlExtension.extract(self.io)
                last_gce = gce
            elif next_two_bytes[0] == "\x2c":
                self.io.seek(-1, SEEK_CUR)
                img = ImageBlock.extract(self.io)
                self.frames.append(dict(gce=last_gce, image_block=img))
            else:
                print('No matching two bytes')
                self.io.seek(-1 * len(next_two_bytes), SEEK_CUR)
                break

        # Trailer
        self.trailer = Trailer(self.io.read(-1))
        return

    @property
    def total_delay(self):
        delays = (f["gce"].delay for f in self.frames)
        return sum(delays)
    
    def reconstruct_bytes(self):
        return self.header.raw + self.screen_desc.raw + self.global_color_table.raw + \
            "".join(x.raw for x in self.application_extensions) + \
            "".join(x.raw for x in self.comment_extensions) + \
            "".join((f["gce"].raw or "") + f["image_block"].raw for f in self.frames) + \
            self.trailer.raw
