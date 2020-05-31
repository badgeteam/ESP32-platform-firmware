import machine, i2c, time

################ I2C Register space mapping ###############
# * Byte	Description
# *
# * 0: 		Interrupt pin reason bitmask (0 = no interrupt, 1 = general, 2 = button state change, 4 = incoming USB MIDI message
# * 1-3:	Reserved
# *
# * 4-5:	Keypad touch state bitmask (little endian, 1 = top left, 32768 = bottom right)
# * 6-9:	Reserved
# *
# * 10-57:	LED PWM values (3-byte RGB per LED, starting from top left, row-wise)
# * 58:		LED dirty byte (0 = data is unchanged, 1 = LEDs should be updated by STM32)
# * 59-63:	Reserved
# *
# * 64:		USB HID key modifier to send over USB
# * 65-70:	USB HID keycodes to send over USB (max 6 concurrent ones)
# * 71:		USB HID keyboard dirty byte (0 = data is unchanged, 1 = keycodes should be sent by STM32 over USB)
# *
# * 72:		USB HID mouse button bitmask (1 = left mouse, 2 = right mouse, 4 = middle mouse, etc)
# * 73:		USB HID mouse relative x movement (signed byte)
# * 74:		USB HID mouse relative y movement (signed byte)
# * 75:		USB HID mouse wheel (signed byte)
# * 76:		USB HID mouse horizontal pan (signed byte)
# * 77:		USB HID mouse dirty byte (0 = data is unchanged, 1 = mouse data should be sent by STM32 over USB)
# *
# * 78-80:	USB MIDI data slot 1 (commonly 0x90 Note On or 0x80 Note Off, then a frequency, then a volume)
# * 81:		USB MIDI dirty byte 1 (0 = data is unchanged, 1 = midi data slot 1 should be sent by STM32 over USB)
# * 82-84:	USB MIDI data slot 2 (commonly 0x90 Note On or 0x80 Note Off, then a frequency, then a volume)
# * 85:		USB MIDI dirty byte 2 (0 = data is unchanged, 1 = midi data slot 2 should be sent by STM32 over USB)
# * 86-88:	USB MIDI data slot 3 (commonly 0x90 Note On or 0x80 Note Off, then a frequency, then a volume)
# * 89:		USB MIDI dirty byte 3 (0 = data is unchanged, 1 = midi data slot 3 should be sent by STM32 over USB)
# * 90-92:	USB MIDI data slot 4 (commonly 0x90 Note On or 0x80 Note Off, then a frequency, then a volume)
# * 93:		USB MIDI dirty byte 4 (0 = data is unchanged, 1 = midi data slot 4 should be sent by STM32 over USB)
# *
# * 94-96:	USB MIDI incoming data
# * 97-128:	Reserved
################ End of I2C Register space mapping ###############

_I2C_ADDR = const(25)
_I2C_RETRIES = const(3)
_OFFSET_I2C_INTERRUPT_REASON = const(0)
_INTERRUPT_PIN = const(0)  # GPIO 0

INTERRUPT_GENERAL = 1
INTERRUPT_KEYPAD = 2
INTERRUPT_MIDI = 4
INTERRUPT_HID_WRITTEN = 8
INTERRUPT_MIDI_WRITTEN = 16

interrupt_mask = 0
interrupt_handlers = {
    INTERRUPT_GENERAL: [],
    INTERRUPT_KEYPAD: [],
    INTERRUPT_MIDI: [],
    INTERRUPT_HID_WRITTEN: [],
    INTERRUPT_MIDI_WRITTEN: []
}

def i2c_read_reg(offset, length):
    for _ in range(_I2C_RETRIES):
        try:
            response = i2c.i2c_read_reg(_I2C_ADDR, offset, length)
            return response
        except Exception as e:
            print('i2c read error', e)
        time.sleep(0.1)
    raise Exception('Failed to read from STM32 over i2c')

def i2c_write_reg(offset, data):
    for _ in range(_I2C_RETRIES):
        try:
            response = i2c.i2c_write_reg(_I2C_ADDR, offset, data)
            return response
        except Exception as e:
            print('i2c write error', e)
        time.sleep(0.1)
    raise Exception('Failed to write to STM32 over i2c')


def add_interrupt_handler(type, handler):
    global interrupt_handlers
    if type not in interrupt_handlers:
        raise Exception('Interrupt type should be one of INTERRUPT_GENERAL, INTERRUPT_KEYPAD, etc.')
    interrupt_handlers[type].append(handler)

def remove_interrupt_handler(type, handler):
    global interrupt_handlers
    if type not in interrupt_handlers:
        raise Exception('Interrupt type should be one of INTERRUPT_GENERAL, INTERRUPT_KEYPAD, etc.')
    interrupt_handlers[type].append(handler)

    for index, _handler in interrupt_handlers[type]:
        if _handler == handler:
            del interrupt_handlers[type][index]
            break

def _get_interrupt_mask():
    global interrupt_mask
    response = i2c_read_reg(_OFFSET_I2C_INTERRUPT_REASON, 1)
    interrupt_mask = int.from_bytes(response, 'little')

def _interrupt_callback(_):
    global keypad_state
    global interrupt_mask
    try:
        _get_interrupt_mask()
    except:
        print('Failed to get interrupt mask')
        return
    for reason, handlers in interrupt_handlers.items():
        if interrupt_mask & reason:           
            for handler in handlers:
                try:
                    handler()
                except Exception as e:
                    import sys
                    sys.print_exception(e)

pin = machine.Pin(_INTERRUPT_PIN, machine.Pin.IN, handler=_interrupt_callback, trigger=machine.Pin.IRQ_FALLING)
try:
    _get_interrupt_mask()
except:
    print('Failed to get interrupt mask')