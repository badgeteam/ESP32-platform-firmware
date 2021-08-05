import stm32

CMD_LCD_MODE      = 0x0001
CMD_LCD_BACKLIGHT = 0x0002
CMD_LED_SET       = 0x0004
CMD_ADC_SAMPLE    = 0x0005
CMD_LED_OFF       = 0x0006
CMD_GPIO_MODE     = 0x0007
CMD_GPIO_SET      = 0x0008
CMD_GPIO_MASK     = 0x0009

def send(command,args=[]):
    data = [command&0xFF,(command>>8)&0xFF] + args
    if len(data) > 18:
        raise ValueError("argument list too long")
    while len(data) < 18:
        data += [0]
    print(data)
    stm32.transaction(bytes(data))

def lcd_mode(mode=False):
    send(CMD_LCD_MODE, [mode&1])

def lcd_backlight(brightness=255):
    send(CMD_LCD_BACKLIGHT, [brightness])

def led_set(position, brightness, red, green, blue, white):
    send(CMD_LED_SET, [position&0xFF, brightness&0xFF, red&0xFF, green&0xFF, blue&0xFF, white&0xFF])

def led_off():
    send(CMD_LED_OFF)

def adc_disable():
    send(CMD_ADC_SAMPLE, [0x00])

def adc_enable():
    send(CMD_ADC_SAMPLE, [0x01])

def adc_single():
    send(CMD_ADC_SAMPLE, [0x02])
