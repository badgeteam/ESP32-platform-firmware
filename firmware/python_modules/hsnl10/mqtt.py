import wifi, _thread
from utime import sleep, sleep_us
from umqtt.simple import MQTTClient
from machine import Pin, nvs_getstr

spaces = {
    'leeuwarden': 0,
    'amsterdam': 1,
    'utrecht': 2,
    'denhaag': 3,
    'rotterdam': 4,
    'zwolle': 5,
    'amersfoort': 6,
    'arnhem': 7,
    'wageningen': 8,
    'eindhoven': 9,
    'enschede': 11,
    'nijmegen': 12,
    'venlo': 13,
    'heerlen': 14
}

leds = [0] * (15 * 2)  # Red and green status for all leds
rows = [ Pin(27, Pin.OUT), Pin(26, Pin.OUT), Pin(32, Pin.OUT), Pin(33, Pin.OUT), Pin(25, Pin.OUT) ]
cols = [
    # Red
    Pin(4, Pin.OUT), Pin(17, Pin.OUT), Pin(18, Pin.OUT),
    # Green
    Pin(16, Pin.OUT), Pin(5, Pin.OUT), Pin(19, Pin.OUT) ]

ssid = nvs_getstr("system", "wifi.ssid") or 'revspace-pub-2.4ghz'
password = nvs_getstr("system", "wifi.password") or ''
wifi.connect(ssid, password)
print('Connecting to WiFi...')
while not wifi.status():
    print('...')
    sleep(1)
print('Connected')

clientname = 'HSNL10'
servername = 'hoera10jaar.revspace.nl'
topic= 'hoera10jaar/#'

def matrix():
    """
    We should probably do this with I2S hardware DMA instead.
    :return:
    """
    while True:
        for col in range(0, 6):
            for row in range(0, 5):
                rows[row].value(leds[col * 5 + row])
            # print('col %d rows %s' % (col, ' '.join([str(leds[col * 5 + row]) for row in range(0, 5)])))
            cols[col].value(1)
            sleep_us(10)
            cols[col].value(0)


def byte_string_to_string(text):
    """
    Required because uPy casts str(b'test') -> "b'test'" for some reason
    :param text:
    :return:
    """
    return str(text).replace('b\'', '').replace('\'', '')

def sub_cb(topic, msg):
    space = byte_string_to_string(topic).split('/')[1]
    status = byte_string_to_string(msg)
    space_index = spaces[space]
    print('[%s] %s (%d) : %s' % (topic, space, space_index, status))
    leds[space_index] = 1 if (status == 'red' or status == 'yellow') else 0
    leds[15 + space_index] = 1 if (status == 'green' or status == 'yellow') else 0

client = MQTTClient(clientname, servername)
client.set_callback(sub_cb)
client.connect()
client.subscribe(topic)
print('MQTT subscribed')

for _ in range(15):
    client.check_msg()
    sleep(0.1)
_thread.start_new_thread('matrix', matrix, (), samecore=False)

while True:
    client.check_msg()
    sleep(1)