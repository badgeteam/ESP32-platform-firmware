# This file is part of the Troopers 19 Badge project, https://troopers.de/troopers19/
#
# The BSD 3-Clause License
#
# Copyright (c) 2019 "Malte Heinzelmann" <malte@hnzlmnn.de>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

# Changed a little bit to fit the BADGE.TEAM firmware!

import time
import machine

from machine import Pin


class PCA95XX:

    ACTIVE = '1'
    INACTIVE = '0'
    UP = '1'
    DOWN = '0'

    def __init__(self, i2c, address, mapping=[], interrupt=None, handler=None, wakeup=False, inverted=False):
        self.inverted = inverted
        self.i2c = i2c
        self.address = address
        self.mapping = mapping
        self.interrupt = interrupt
        self.handler = handler
        self.state = self.INACTIVE * 16 # Initialize all buttons as not connected
        self.wakeup = wakeup

    def init(self):
        if self.interrupt:
            self.interrupt.init(mode=Pin.IN, trigger=Pin.IRQ_FALLING, handler=self._on_input)
            #if self.wakeup:
            #    self.interrupt.irq(trigger=Pin.WAKE_LOW, wake=machine.SLEEP | machine.DEEPSLEEP)
            # To initialize the interrupts read from the chip once
            self.state = self.read_state()
            # Fix bug that only registers interrupts after first reset
            self.state = self.read_state()

    def read_state(self):
        raise NotImplementedError('How should I read from the IO Expander?')

    def get_events(self):
        # time.sleep_ms(30)
        state = self.read_state()
        changes = []
        for i in range(min(len(self.state), len(state))):
            try:
                if self.state[i] != state[i]:
                    if state[i] == PCA95XX.UP:
                        # Button just went up
                        changes.append((self.mapping[i], not self.inverted))
                    elif state[i] == PCA95XX.DOWN:
                        # Button just went down
                        changes.append((self.mapping[i], self.inverted))
            except IndexError:
                # If there is no mapping just ignore the key ¯\_(ツ)_/¯
                pass
        self.state = state
        return changes

    def is_pressed(self, code):
        for i, key in enumerate(self.mapping):
            if key == code:
                return self.state[i] == self.ACTIVE
        return False


    def _on_input(self, pin):
        if self.handler:
            for event in self.get_events():
                try:
                    self.handler(*event)
                except KeyboardInterrupt:
                    raise

    def close(self):
        self.interrupt.irq(handler=None)

