"""
Disobey 2020
Badge flashing script

Author: Heikki Juva
Email: heikki@juva.lu

Based on batchflash.py-script by Renze.
Original script was depending on the information of serial device being
disconnected at the end of flashing process. For flashing Disobey 2020 badges
we utilize the TAG-connector for flashing and debugging. This makes the
flashing process easier, as the connector is designed for easy and reliable
temporary connection, with fast and easy connect and disconnect. These
connectors are wired to FTDIs FT4232 quad- USART chip. This chip presents
itself as four separate serial devices over single USB connection. It includes
all the normal control lines, so it is compatible with the reset-scheme used
with esptool.

As the serial device is always present, the badge removal has to be based on
separate input. I chose to utilize the numeric keypad in the flashing terminals
(Disobey 2020 Badge Terminal).

The application takes list of serial devices as an input, and uses the keypad
numbers to trigger flashing on corresponding port. It is adviced to mark the
connectors with corresponding numbers. This scheme works equally well with
separate USB-serial adapters.
"""

from os import system
import argparse

import pygame
import sys
from pygame.locals import *
import subprocess
import threading
import concurrent.futures
import time
import random

# Define Colours
WHITE = (255, 255, 255)
GREEN = (0, 255, 0)
RED = (255, 0, 0)
BLUE = (0, 0, 255)
BLACK = (0, 0, 0)
FUCHSIA = (255, 0, 255)
GRAY = (128, 128, 128)
LIME = (0, 128, 0)
MAROON = (128, 0, 0)
NAVYBLUE = (0, 0, 128)
OLIVE = (128, 128, 0)
PURPLE = (128, 0, 128)
TEAL = (0, 128, 128)

TRIGGER_KEYS = [
    pygame.K_1,
    pygame.K_2,
    pygame.K_3,
    pygame.K_4,
    pygame.K_5,
    pygame.K_6,
    pygame.K_7,
    pygame.K_8,
    pygame.K_9
]

TEST = False
THING = False

class Flasher:

    def flash_thing(self, port):
        esptool_path = "esp-idf/components/esptool_py/esptool/esptool.py"
        firmware_folder = 'firmware/build'

        # Erase Flash
        assert system(
            'python2 ' + esptool_path +
            ' --chip esp32' +
            ' --port ' + port +
            ' --baud 2000000' +
            ' --before default_reset' +
            ' --after hard_reset erase_flash'
        ) == 0, "Failed to erase flash"

        # Write Flash
        assert system(
            'python2 ' + esptool_path +
            ' --chip esp32' +
            ' --port ' + port +
            ' --baud 1000000' +
            ' --before default_reset' +
            ' --after hard_reset write_flash' +
            ' 0x1000' +
            ' ' + firmware_folder + '/thing_firmware.bin'
        ) == 0, "Failed to erase flash"

    def flash_badge(self, port):
        esptool_path = "esp-idf/components/esptool_py/esptool/esptool.py"
        firmware_folder = 'firmware/build'

        """
        # Flash OTA data
        assert subprocess.call([
            "python2",
            esptool_path,
            '--chip', 'esp32',
            '--port', port,
            '--baud', '2000000',
            '--before', 'default_reset',
            '--after', 'hard_reset', 'write_flash',
            '-z',
            '--flash_mode', 'dio',
            '--flash_freq', '80m',
            '--flash_size', 'detect', '0xd000',
            firmware_folder + '/ota_data_initial.bin'
        ]) == 0, "Failed to flash OTA data"
        """

        # Erase Flash
        assert system(
            esptool_path +
            ' --chip esp32' +
            ' --port ' + port +
            ' --baud 2000000' +
            ' --before default_reset' +
            ' --after hard_reset erase_flash'
        ) == 0, "Failed to erase flash"

        # Flash Firmware
        assert system(
            esptool_path +
            ' --chip esp32' +
            ' --port ' + port +
            ' --baud 2000000' +
            ' --before default_reset' +
            ' --after hard_reset write_flash' +
            ' -z' +
            ' --flash_mode dio' +
            ' --flash_freq 80m' +
            ' --flash_size detect 0xd000' +
            ' ' + firmware_folder + '/ota_data_initial.bin'
        ) == 0, "Failed to flash OTA data"

        # Flash Firmware
        assert system(
            esptool_path +
            ' --chip esp32' +
            ' --port ' + port +
            ' --baud 2000000' +
            ' --before default_reset' +
            ' --after hard_reset write_flash' +
            ' -z' +
            ' --flash_mode dio' +
            ' --flash_freq 80m' +
            ' --flash_size detect 0x10000' +
            ' ' + firmware_folder + '/firmware.bin'
        ) == 0, "Failed to flash firmware"

        # Flash Bootloader
        assert system(
            esptool_path +
            ' --chip esp32' +
            ' --port ' + port +
            ' --baud 2000000' +
            ' --before default_reset' +
            ' --after hard_reset write_flash' +
            ' -z' +
            ' --flash_mode dio' +
            ' --flash_freq 80m' +
            ' --flash_size detect 0x1000' +
            ' ' + firmware_folder + '/bootloader/bootloader.bin'
        ) == 0, "Failed to flash Bootloader"

        # Flash partition table
        assert system(
            esptool_path +
            ' --chip esp32' +
            ' --port ' + port +
            ' --baud 2000000' +
            ' --before default_reset' +
            ' --after hard_reset write_flash' +
            ' -z' +
            ' --flash_mode dio' +
            ' --flash_freq 80m' +
            ' --flash_size detect 0x8000' +
            ' ' + firmware_folder + '/generic_4MB.bin'
        ) == 0, "Failed to flash partition table"

    def flash_badge2(self, port):
        esptool_path = "esp-idf/components/esptool_py/esptool/esptool.py"
        firmware_folder = 'firmware/build'

        # Erase Flash
        assert system(
            esptool_path +
            ' --chip esp32' +
            ' --port ' + port +
            ' --baud 2000000' +
            ' --before default_reset' +
            ' --after hard_reset erase_flash'
        ) == 0, "Failed to erase flash"

        # Flash Firmware
        assert system(
            esptool_path +
            ' --chip esp32' +
            ' --port ' + port +
            ' --baud 2000000' +
            ' --before default_reset' +
            ' --after hard_reset write_flash' +
            ' -z' +
            ' --flash_mode dio' +
            ' --flash_freq 80m' +
            ' --flash_size detect' +
            ' 0xd000 ' + firmware_folder + '/ota_data_initial.bin' +
            ' 0x1000 ' + firmware_folder + '/bootloader/bootloader.bin' +
            ' 0x8000 ' + firmware_folder + '/sha2017_16MB.bin' +
            ' 0x10000 ' + firmware_folder + '/firmware.bin' +
            ' 0x191000 initial_files_disobey2020.zip'
        ) == 0, "Failed to flash badge"

    def flash_dummy(self, port, fail=False):
        time.sleep(1)
        if fail:
            assert False

    def run_flashing(self, port_data):
        self.status_texts[port_data["id"]]["header"] = \
            self.header_font.render(str(port_data["id"] + 1), True, (BLUE))
        self.status_texts[port_data["id"]]["running"] = True
        time.sleep(0.2)
        self.status_texts[port_data["id"]]["header"] = \
            self.header_font.render(str(port_data["id"] + 1), True, (GREEN))
        try:
            if TEST:
                self.flash_dummy(
                    port_data["port"],
                    fail=random.choice([True, False])
                )
            if THING:
                self.flash_thing(port_data["port"])
            else:
                self.flash_badge2(port_data["port"])
        except AssertionError:
            self.status_texts[port_data["id"]]["header"] = \
                self.header_font.render(str(port_data["id"] + 1), True, (RED))
        else:
            self.status_texts[port_data["id"]]["header"] = \
                self.header_font.render(str(port_data["id"] + 1), True, (GRAY))
        finally:
            self.status_texts[port_data["id"]]["running"] = False

    def update_screen(self):
        self.windowSurface.fill(BLACK)
        x = 0
        y_header = 0
        y_name = y_header + 80
        dx = self.windowSurface.get_width() / len(self.status_texts)
        for text in self.status_texts:
            self.windowSurface.blit(text["header"], (x, y_header))
            self.windowSurface.blit(text["name"], (x, y_name))
            x += dx
        pygame.display.update()

    def __init__(self, ports):
        pygame.init()
        WIDTH = 800
        HEIGHT = 600
        self.windowSurface = pygame.display.set_mode((WIDTH, HEIGHT), 0, 32)

        self.header_font = pygame.font.SysFont("Arial", 72)
        self.name_font = pygame.font.SysFont("Arial", 15)

        self.status_texts = []
        for i, port in enumerate(ports):
            colour = GRAY
            id_text = self.header_font.render(str(i + 1), True, (colour))
            port_text = self.name_font.render(port, True, (colour))
            self.status_texts.append({
                "id": i,
                "header": id_text,
                "name": port_text,
                "port": port,
                "running": False
            })

        while True:
            for event in pygame.event.get():
                if event.type == QUIT:
                    pygame.quit()
                    sys.exit()
                if event.type == KEYDOWN:
                    key = event.key
                    if key in TRIGGER_KEYS:
                        i = TRIGGER_KEYS.index(key)
                        if len(self.status_texts) > i:
                            if not self.status_texts[i]["running"]:
                                t = threading.Thread(
                                    target=self.run_flashing,
                                    args=(self.status_texts[i],)
                                )
                                t.start()
            self.update_screen()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "ports",
        metavar="ports",
        type=str,
        nargs='+',
        help="badge serial devices"
    )

    args = parser.parse_args()
    f = Flasher(ports=args.ports)
