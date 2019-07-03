import gc
import machine, time, badge

# Basics for receiving:
# * Startup by enabling the IR receiver to eat power
# * Enable an interrupt (hardware) for receiving the initial pulse
# * When the interrupt is fired disable the rest of the interrups for the duration of this read
# * Read the pulses with time_pulse_us (it's extremely fast)
# * Store this in a buffer with a fixed size, (256 empty arrays)
# * Set a timer to decode the data instantly after interrupts are enabled again.
# * a class can inherit this class and just define a decoder and specify a send pattern 
# * if a send pattern is not the way to go (some totally different protocols) you can just implement another tx function

class BadgeIr():
	freq = 38000
	ticks = 500
	start = 4000
	rxpin = 18
	txpin = 19
	rxenablepin = 21
	rxtimer = None
	readcallback = None
	bitform = []
	pwm_tx = None
	
	def __init__(self, pins = None):
		if pins:
			self.rxpin = pins[0]
			self.txpin = pins[1]
			self.rxenablepin = pins[3]
		elif badge.deviceType == 'Hackerhotel_2019':
			self.rxpin = 12
			self.txpin = 14
			self.rxenablepin = -1
		elif badge.deviceType == 'Disobey_2019':
			self.rxpin = 18
			self.txpin = 19
			self.rxenablepin = 21
		else:
			raise("Please provide pin numbers [rx, tx, rxEnable], enter -1 for rxEnable if it is not used!")
	def initbuffer(self):
		self.buffer=[[]] * 256
		self.bufpos = 0
	# If it returns 1 the buffer gets emptied out and initialized.
	# This is the default, protocol implementations should choose if they want to.
	def decoder(self):
		return 1
	def real_decoder(self,timer):
		self.rxtimer.deinit()
		self.rxtimer = None
		if self.decoder():
			self.initbuffer()
	def cleanbuffer(self,i):
		self.buffer=self.buffer[i:256]+[[]]*i
		self.bufpos-=i
		gc.collect()
	def mr(self):
		if self.bufpos == len(self.buffer):
			return 0
		waitfor=0 if self.pin_rx.value() else 1
		while True:
			t=machine.time_pulse_us(self.pin_rx,waitfor,50*1000)
			if t<0:
				if t == -2:
					return 1
				return 0
			elif t>0:
				waitfor = 0 if waitfor else 1
				self.buffer[self.bufpos]=[waitfor,round(t/self.ticks)]
				self.bufpos+=1
				if self.bufpos == len(self.buffer):
					return 1
	def callback(self,pin):
		irqs = machine.disable_irq()
		hasdata = self.mr()
		machine.enable_irq(irqs)
		if hasdata and not self.rxtimer:
			self.rxtimer = machine.Timer(1)
			self.rxtimer.init(mode=machine.Timer.ONE_SHOT, period=1,callback=self.real_decoder)
	def rx_enable(self):
		if self.rxenablepin >= 0:
			self.pin_rx_enable = machine.Pin(self.rxenablepin, machine.Pin.OUT)
		self.pin_rx        = machine.Pin(self.rxpin, machine.Pin.IN, machine.Pin.PULL_UP)
		self.initbuffer()
		if self.rxenablepin >= 0:
			self.pin_rx_enable.value(True)
		self.pin_rx.irq(trigger=machine.Pin.IRQ_FALLING, handler=self.callback)
	def rx_disable(self):
		self.pin_rx.irq(trigger=0, handler=self.callback)
		if self.rxenablepin >= 0:
			self.pin_rx_enable.value(False)
	def tx_enable(self):
		if not self.pwm_tx:
			self.pin_tx        = machine.Pin(self.txpin, machine.Pin.OUT)
			self.pwm_tx        = machine.PWM(self.pin_tx, freq = self.freq, duty = 0)
	def tx_disable(self):
		self.pwm_tx.duty(0)
	def tx_setduty(self,duty):
		self.pwm_tx.duty(512 * duty)
	def txBit(self,bit):
		for (onoff,tijd) in self.bitform[bit]:
			self.tx_setduty(onoff)
			time.sleep_us(tijd)
	def txByte(self,byte):
		for bit in range(8):
				self.txBit( ( byte >> ( 7 - bit ) ) & 1 ) # MSB

class NecIR(BadgeIr):
	# Implements NEC Infrared 
	# Example:
	#    IR=NecIR()
	#    NecIR.command= <function (address,command)>
	#    NecIR.repeat=  <function ()>
	#    NecIR.rx_enable()
	#  To stop receiving:
	#    NecIR.rx_disable()
	#  To send:
	#    NecIR.tx(<byte address>,<byte command>)
	#    NecIR.tx_repeat()
	command = None
	repeat = None
	bitform = { 0: [[1,562],[0,562]], 1: [[1,562],[0,1687]], 's': [[1,9000],[0,4500]], 'e': [[1,562],[0,100]], 'r': [[1,9000],[0,2500],[1,562],[0,100]] }

	def tx(self,addr,cmd):
		self.tx_enable()
		self.txBit('s')
		self.txByte(addr)
		self.txByte(addr ^ 0xFF)
		self.txByte(cmd)
		self.txByte(cmd ^ 0xFF)
		self.txBit('e')
		self.tx_disable()

	def tx_repeat(self):
		self.txBit('r')

	def decoder(self):
		decoded=0
		i=0
		while True and self.bufpos-i>0:
			(val,time)=self.buffer[i]
			i+=1
			if val==0 and time==9:
				if self.bufpos<66: return(0) # Not yet complete....
				p1=None
				p2=None
				bits=0
				while True and self.bufpos-i>0:
					(val,time)=self.buffer[i]
					i+=1
					if time>0:
						if p1==None:
							p1=(val,time)
							if bits==32 and p1[1]==1:
								self.cleanbuffer(i)
								if (decoded >> 24 & 0xFF) == (0xFF ^ (decoded >> 16 & 0xFF)) and (decoded >> 8 & 0xFF) == (0xFF ^ (decoded >> 0 & 0xFF)) and self.command:
									self.command(decoded >> 24 & 0xFF,decoded >> 8 & 0xFF)
								return(0)
						else:
							p2=(val,time)
							if p1[1]==1 and p2[1]==3:
								decoded=decoded<<1 | 1
								bits+=1
							elif p1[1]==1 and p2[1]==1:
								decoded=decoded<<1
								bits+=1
						if bits==32 and p2==None:
							self.cleanbuffer(i)
							return(0)
						p1=None
						p2=None
					elif time<0:
						self.cleanbuffer(i)
						return(0)
			elif val==1 and time==18:
				if self.buffer[i] == [0,4] and self.buffer[i+1] == [1,1]:
					i+=2
					if self.repeat: self.repeat()
					return(0)
		self.cleanbuffer(i)
		return(0)
