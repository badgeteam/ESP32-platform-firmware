---
title: "WiFi"
nodateline: true
weight: 60
---

The *wifi* API allows you to connect to WiFi networks easily.

# Reference
| Command    | Parameters                     | Description                                                                                                                                                                                                                                                                                                                     |
| ---------- | ------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| connect    | \[ssid\], \[password\]         | Connect to a WiFi network. By default the stored credentials are used, but you can optionally provide the SSID (and password) of the network to connect to.                                                                                                                                                                     |
| disconnect | \-                             | Disconnect from the WiFi network.                                                                                                                                                                                                                                                                                               |
| status     | \-                             | Returns True if connected and False if not connected.                                                                                                                                                                                                                                                                           |
| wait       | \[timeout\]                    | Wait until a connection with the WiFi network has been made or until the timeout time is reached. Timeout is in seconds but may be provided in 10ths of seconds. If no timeout is provided the default timeout is used. Returns True if connected after waiting and False if a connection could not be made before the timeout. |
| ntp        | \[only-if-needed\], \[server\] | Synchronize the Real-Time-Clock with the network. Normally the synchronisation is only started when the system clock has not yet been set since the last reset. This can be overruled by setting the only-if-needed parameter to False. By default the "'pool.ntp.org" server pool is used.                                     |

# Usage
```
import wifi
wifi.connect() # Connect to the WiFi network using the stored credentials
if not wifi.wait():
	print("Unable to connect to the WiFi network.")
else:
	print("You are now connected to WiFi!")
```

# Wait, is that all you can do with WiFi?!
No, of course not. The whole network API from the mainline MicroPython project is available on the badge.team firmware.
Here are some examples for doing the stuff you're probably looking for:

## Connecting to a WiFi network, the hard way...
```
import network, machine, time

# First we fetch the stored WiFi credentials
ssid = machine.nvs_getstr("system", "wifi.ssid")
password = machine.nvs_getstr("system", "wifi.password")

# Create the station (WiFi client) interface
sta_if = network.WLAN(network.STA_IF)

# Activate the station interface
sta_if.active(True)

# Connect using the credentials
if ssid and password:
	sta_if.connect(ssid, password) # Secured WiFi network
elif ssid: # Password is None
	sta_if.connect(ssid) # Open WiFi network
else:
	print("ERROR: no SSID provided. Please configure WiFi (or manually set the variables at the top of this example)")

wait = 50 # 50x 100ms = 5 seconds
while not sta_if.isconnected() and wait > 0:
	wait -= 1
	time.sleep(0.1) # Wait 100ms

if sta_if.isconnected():
	print("Connected!")
	ip_addr, netmask, gateway, dns_server = sta_if.ifconfig()
	print("My IP address is '{}', with netmask '{}'.".format(ip_addr, netmask))
	print("The gateway is at '{}' and the DNS server is at '{}'.".format(gateway, dns_server))
else:
	print("Failed to connect to the WiFi network.")
```

## Scanning for networks
```
import network
sta_if = network.WLAN(network.STA_IF)
sta_if.active(True)
data = sta_if.scan()
for item in data:
	print("SSID: {}, BSSID: {}. CHANNEL: {}, RSSI: {}, AUTHMODE: {} / {}, HIDDEN: {}".format(item[0], item[1], item[2], item[3], item[4], item[5], item[6]))
```
## Creating an access-point

```
import network
ap_if = network.WLAN(network.AP_IF)
ap_if.config(essid="badgeNetwork", authmode=network.AUTH_WPA2_PSK, password="helloworld") # Create a network called "badgeNetwork" with password "helloworld"
ap_if.active(True)
```

Note: if you get "unknown error 0x000b" after running the config command then the password you chose is too short.

## More information
We used the [loboris micropython fork (<- link)](https://github.com/loboris/MicroPython_ESP32_psRAM_LoBo/wiki/network) as the core of our badge firmware. The network API comes directly from his project.


The API looks a lot like the [official MicroPython network API (<- link)](https://docs.micropython.org/en/latest/library/network.html).
