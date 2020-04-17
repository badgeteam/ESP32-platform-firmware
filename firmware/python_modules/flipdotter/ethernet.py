import network, time, machine
l = network.LAN(mdc = machine.Pin(23), mdio = machine.Pin(18), power = machine.Pin(12), phy_type = network.PHY_LAN8720, phy_addr=1, clk_type=3)
l.active(True)

def status():
	return l.isconnected() and l.hasip()

def connected():
	return l.isconnected() 

def hasip():
	return l.hasip()

def ifconfig():
	return l.ifconfig()
