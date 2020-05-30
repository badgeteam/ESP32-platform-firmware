# The MIT License (MIT)
#
# Copyright (c) 2017 Dan Halbert
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

# The ASCII_TO_KEYCODE bytes object is used as a table to maps ASCII 0-127
# to the corresponding # keycode on a US 104-key keyboard.
# The user should not normally need to use this table,
# but it is not marked as private.
#
# Because the table only goes to 127, we use the top bit of each byte (ox80) to indicate
# that the shift key should be pressed. So any values 0x{8,9,a,b}* are shifted characters.
#
# The Python compiler will concatenate all these bytes literals into a single bytes object.
# Micropython/CircuitPython will store the resulting bytes constant in flash memory
# if it's in a .mpy file, so it doesn't use up valuable RAM.
#
# \x00 entries have no keyboard key and so won't be sent.
SHIFT_FLAG = 0x80
_ASCII_TO_KEYCODE = (
    b"\x00"  # NUL
    b"\x00"  # SOH
    b"\x00"  # STX
    b"\x00"  # ETX
    b"\x00"  # EOT
    b"\x00"  # ENQ
    b"\x00"  # ACK
    b"\x00"  # BEL \a
    b"\x2a"  # BS BACKSPACE \b (called DELETE in the usb.org document)
    b"\x2b"  # TAB \t
    b"\x28"  # LF \n (called Return or ENTER in the usb.org document)
    b"\x00"  # VT \v
    b"\x00"  # FF \f
    b"\x00"  # CR \r
    b"\x00"  # SO
    b"\x00"  # SI
    b"\x00"  # DLE
    b"\x00"  # DC1
    b"\x00"  # DC2
    b"\x00"  # DC3
    b"\x00"  # DC4
    b"\x00"  # NAK
    b"\x00"  # SYN
    b"\x00"  # ETB
    b"\x00"  # CAN
    b"\x00"  # EM
    b"\x00"  # SUB
    b"\x29"  # ESC
    b"\x00"  # FS
    b"\x00"  # GS
    b"\x00"  # RS
    b"\x00"  # US
    b"\x2c"  # SPACE
    b"\x9e"  # ! x1e|SHIFT_FLAG (shift 1)
    b"\xb4"  # " x34|SHIFT_FLAG (shift ')
    b"\xa0"  # # x20|SHIFT_FLAG (shift 3)
    b"\xa1"  # $ x21|SHIFT_FLAG (shift 4)
    b"\xa2"  # % x22|SHIFT_FLAG (shift 5)
    b"\xa4"  # & x24|SHIFT_FLAG (shift 7)
    b"\x34"  # '
    b"\xa6"  # ( x26|SHIFT_FLAG (shift 9)
    b"\xa7"  # ) x27|SHIFT_FLAG (shift 0)
    b"\xa5"  # * x25|SHIFT_FLAG (shift 8)
    b"\xae"  # + x2e|SHIFT_FLAG (shift =)
    b"\x36"  # ,
    b"\x2d"  # -
    b"\x37"  # .
    b"\x38"  # /
    b"\x27"  # 0
    b"\x1e"  # 1
    b"\x1f"  # 2
    b"\x20"  # 3
    b"\x21"  # 4
    b"\x22"  # 5
    b"\x23"  # 6
    b"\x24"  # 7
    b"\x25"  # 8
    b"\x26"  # 9
    b"\xb3"  # : x33|SHIFT_FLAG (shift ;)
    b"\x33"  # ;
    b"\xb6"  # < x36|SHIFT_FLAG (shift ,)
    b"\x2e"  # =
    b"\xb7"  # > x37|SHIFT_FLAG (shift .)
    b"\xb8"  # ? x38|SHIFT_FLAG (shift /)
    b"\x9f"  # @ x1f|SHIFT_FLAG (shift 2)
    b"\x84"  # A x04|SHIFT_FLAG (shift a)
    b"\x85"  # B x05|SHIFT_FLAG (etc.)
    b"\x86"  # C x06|SHIFT_FLAG
    b"\x87"  # D x07|SHIFT_FLAG
    b"\x88"  # E x08|SHIFT_FLAG
    b"\x89"  # F x09|SHIFT_FLAG
    b"\x8a"  # G x0a|SHIFT_FLAG
    b"\x8b"  # H x0b|SHIFT_FLAG
    b"\x8c"  # I x0c|SHIFT_FLAG
    b"\x8d"  # J x0d|SHIFT_FLAG
    b"\x8e"  # K x0e|SHIFT_FLAG
    b"\x8f"  # L x0f|SHIFT_FLAG
    b"\x90"  # M x10|SHIFT_FLAG
    b"\x91"  # N x11|SHIFT_FLAG
    b"\x92"  # O x12|SHIFT_FLAG
    b"\x93"  # P x13|SHIFT_FLAG
    b"\x94"  # Q x14|SHIFT_FLAG
    b"\x95"  # R x15|SHIFT_FLAG
    b"\x96"  # S x16|SHIFT_FLAG
    b"\x97"  # T x17|SHIFT_FLAG
    b"\x98"  # U x18|SHIFT_FLAG
    b"\x99"  # V x19|SHIFT_FLAG
    b"\x9a"  # W x1a|SHIFT_FLAG
    b"\x9b"  # X x1b|SHIFT_FLAG
    b"\x9c"  # Y x1c|SHIFT_FLAG
    b"\x9d"  # Z x1d|SHIFT_FLAG
    b"\x2f"  # [
    b"\x31"  # \ backslash
    b"\x30"  # ]
    b"\xa3"  # ^ x23|SHIFT_FLAG (shift 6)
    b"\xad"  # _ x2d|SHIFT_FLAG (shift -)
    b"\x35"  # `
    b"\x04"  # a
    b"\x05"  # b
    b"\x06"  # c
    b"\x07"  # d
    b"\x08"  # e
    b"\x09"  # f
    b"\x0a"  # g
    b"\x0b"  # h
    b"\x0c"  # i
    b"\x0d"  # j
    b"\x0e"  # k
    b"\x0f"  # l
    b"\x10"  # m
    b"\x11"  # n
    b"\x12"  # o
    b"\x13"  # p
    b"\x14"  # q
    b"\x15"  # r
    b"\x16"  # s
    b"\x17"  # t
    b"\x18"  # u
    b"\x19"  # v
    b"\x1a"  # w
    b"\x1b"  # x
    b"\x1c"  # y
    b"\x1d"  # z
    b"\xaf"  # { x2f|SHIFT_FLAG (shift [)
    b"\xb1"  # | x31|SHIFT_FLAG (shift \)
    b"\xb0"  # } x30|SHIFT_FLAG (shift ])
    b"\xb5"  # ~ x35|SHIFT_FLAG (shift `)
    b"\x4c"  # DEL DELETE (called Forward Delete in usb.org document)
)

CAPS_LOCK = 0x39
"""Caps Lock"""

F1 = 0x3A
"""Function key F1"""
F2 = 0x3B
"""Function key F2"""
F3 = 0x3C
"""Function key F3"""
F4 = 0x3D
"""Function key F4"""
F5 = 0x3E
"""Function key F5"""
F6 = 0x3F
"""Function key F6"""
F7 = 0x40
"""Function key F7"""
F8 = 0x41
"""Function key F8"""
F9 = 0x42
"""Function key F9"""
F10 = 0x43
"""Function key F10"""
F11 = 0x44
"""Function key F11"""
F12 = 0x45
"""Function key F12"""

PRINT_SCREEN = 0x46
"""Print Screen (SysRq)"""
SCROLL_LOCK = 0x47
"""Scroll Lock"""
PAUSE = 0x48
"""Pause (Break)"""

INSERT = 0x49
"""Insert"""
HOME = 0x4A
"""Home (often moves to beginning of line)"""
PAGE_UP = 0x4B
"""Go back one page"""
DELETE = 0x4C
"""Delete forward"""
END = 0x4D
"""End (often moves to end of line)"""
PAGE_DOWN = 0x4E
"""Go forward one page"""

RIGHT_ARROW = 0x4F
"""Move the cursor right"""
LEFT_ARROW = 0x50
"""Move the cursor left"""
DOWN_ARROW = 0x51
"""Move the cursor down"""
UP_ARROW = 0x52
"""Move the cursor up"""

KEYPAD_NUMLOCK = 0x53
"""Num Lock (Clear on Mac)"""
KEYPAD_FORWARD_SLASH = 0x54
"""Keypad ``/``"""
KEYPAD_ASTERISK = 0x55
"""Keypad ``*``"""
KEYPAD_MINUS = 0x56
"""Keyapd ``-``"""
KEYPAD_PLUS = 0x57
"""Keypad ``+``"""
KEYPAD_ENTER = 0x58
"""Keypad Enter"""
KEYPAD_ONE = 0x59
"""Keypad ``1`` and End"""
KEYPAD_TWO = 0x5A
"""Keypad ``2`` and Down Arrow"""
KEYPAD_THREE = 0x5B
"""Keypad ``3`` and PgDn"""
KEYPAD_FOUR = 0x5C
"""Keypad ``4`` and Left Arrow"""
KEYPAD_FIVE = 0x5D
"""Keypad ``5``"""
KEYPAD_SIX = 0x5E
"""Keypad ``6`` and Right Arrow"""
KEYPAD_SEVEN = 0x5F
"""Keypad ``7`` and Home"""
KEYPAD_EIGHT = 0x60
"""Keypad ``8`` and Up Arrow"""
KEYPAD_NINE = 0x61
"""Keypad ``9`` and PgUp"""
KEYPAD_ZERO = 0x62
"""Keypad ``0`` and Ins"""
KEYPAD_PERIOD = 0x63
"""Keypad ``.`` and Del"""
KEYPAD_BACKSLASH = 0x64
"""Keypad ``\\`` and ``|`` (Non-US)"""

APPLICATION = 0x65
"""Application: also known as the Menu key (Windows)"""
POWER = 0x66
"""Power (Mac)"""
KEYPAD_EQUALS = 0x67
"""Keypad ``=`` (Mac)"""
F13 = 0x68
"""Function key F13 (Mac)"""
F14 = 0x69
"""Function key F14 (Mac)"""
F15 = 0x6A
"""Function key F15 (Mac)"""
F16 = 0x6B
"""Function key F16 (Mac)"""
F17 = 0x6C
"""Function key F17 (Mac)"""
F18 = 0x6D
"""Function key F18 (Mac)"""
F19 = 0x6E
"""Function key F19 (Mac)"""

LEFT_CONTROL = 0xE0
"""Control modifier left of the spacebar"""
CONTROL = LEFT_CONTROL
"""Alias for LEFT_CONTROL"""
LEFT_SHIFT = 0xE1
"""Shift modifier left of the spacebar"""
SHIFT = LEFT_SHIFT
"""Alias for LEFT_SHIFT"""
LEFT_ALT = 0xE2
"""Alt modifier left of the spacebar"""
ALT = LEFT_ALT
"""Alias for LEFT_ALT; Alt is also known as Option (Mac)"""
OPTION = ALT
"""Labeled as Option on some Mac keyboards"""
LEFT_GUI = 0xE3
"""GUI modifier left of the spacebar"""
GUI = LEFT_GUI
"""Alias for LEFT_GUI; GUI is also known as the Windows key, Command (Mac), or Meta"""
WINDOWS = GUI
"""Labeled with a Windows logo on Windows keyboards"""
COMMAND = GUI
"""Labeled as Command on Mac keyboards, with a clover glyph"""
RIGHT_CONTROL = 0xE4
"""Control modifier right of the spacebar"""
RIGHT_SHIFT = 0xE5
"""Shift modifier right of the spacebar"""
RIGHT_ALT = 0xE6
"""Alt modifier right of the spacebar"""
RIGHT_GUI = 0xE7
"""GUI modifier right of the spacebar"""

MOD_LEFT_CONTROL = 0x01
MOD_LEFT_SHIFT = 0x02
MOD_LEFT_ALT = 0x04
MOD_LEFT_WIN = 0x08
MOD_RIGHT_CONTROL = 0x10
MOD_RIGHT_SHIFT = 0x20
MOD_RIGHT_ALT = 0x40
MOD_RIGHT_WIN = 0x80
MOD_SHIFT = MOD_LEFT_SHIFT

def char_to_keycode(character):
    ascii = ord(character)
    keycode = _ASCII_TO_KEYCODE[ascii]
    shift_needed = keycode & SHIFT_FLAG
    return (keycode & ~SHIFT_FLAG, shift_needed)