import network, time

print("Configuring interface...")
l = network.LAN(mdc = machine.Pin(23), mdio = machine.Pin(18), power = machine.Pin(12), phy_type = network.PHY_LAN8720, phy_addr=0, clk_type=3)

print("Enabling interface...")
l.active(True)

print("Waiting for connection...")
while not l.isconnected():
	time.sleep(0.1)

print("Waiting for ip address...")
while not l.hasip():
	time.sleep(0.1)

print("Ifconfig:",l.ifconfig())
