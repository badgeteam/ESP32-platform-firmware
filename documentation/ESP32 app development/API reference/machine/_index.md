---
title: "Machine"
nodateline: true
weight: 200
---

The *machine* API makes it possible to access certain hardware interfaces directly, allowing for example direct control of GPIOs, busses (I2C) and other interfaces.

This API is variation on the standard MicroPython *machine* API which has been extended and modified.

{{% notice warning %}}
Not all features described in the official MicroPython documentation are available on the BADGE.TEAM platform firmware. And additionally some functions will differ in syntax from the official MicroPython for ESP32 firmware.
{{% /notice %}}

# [<i class="fa fa-hdd" aria-hidden="true"></i> Non Volitile Storage (NVS)](nvs)

The [NVS functions](nvs) allow for storage and retrieval of small amounts of data to be stored. This API is used to access WiFi credentials and other system information and can be used to manipulate system settings as well as for storing settings specific to your app.

# [<i class="fa fa-microchip" aria-hidden="true"></i> Direct GPIO control](pin)

The *Pin* API can be used to directly control GPIOs of your badge.

# [<i class="fa fa-plug" aria-hidden="true"></i> I2C bus](i2c)
The *machine* API for I2C allows you to control the system I2C bus of your badge, the I2C bus exposed on the SAO, Grove, Qwiic or other extension ports as well as a second I2C bus on any two broken out GPIOs of your choice.

# [<i class="fa fa-plug" aria-hidden="true"></i> SPI bus](#)
Direct control over the SPI bus is currently not supported on the BADGE.TEAM platform firmware. Sorry!
