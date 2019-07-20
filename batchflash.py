#!/usr/local/bin/python

import sys, threading, time
from os import system
from os.path import exists

def flash_daemon(number):
    device = '/dev/ttyUSB%d' % number
    print('Starting flash thread for %s' % device)
    sys.stdout.flush()
    while True:
        if exists(device):
            print('Flashing %s' % device)
            sys.stdout.flush()
            system('env CONFIG_ESPTOOLPY_PORT_X=%s ./erase.sh' % device)
            system('env CONFIG_ESPTOOLPY_PORT_X=%s ./flash.sh' % device)

            # Give system time to adjust
            time.sleep(1)

            # Wait for device to detach
            while exists(device):
                time.sleep(0.1)

        time.sleep(0.1)

for i in range(0,10):
    threading.Thread(target=flash_daemon, args=(i,)).start()

while True:
    time.sleep(1)
# flash_daemon(0)