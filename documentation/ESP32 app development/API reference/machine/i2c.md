# The I2C bus
The *machine* API for I2C allows you to control the system I2C bus of your badge, the I2C bus exposed on the SAO, Grove, Qwiic or other extension ports as well as a second I2C bus on any two broken out GPIOs of your choice.

The ESP32 has two I2C controllers, each of which can be set to master or slave mode. Most of our badges use one of these I2C controllers for their internal I2C bus.
You can take control over this system I2C bus using the machine API without directly causing issues but be adviced that doing this might possibly disrupt communications with one or more system components like the touch-button controller IC or the display.

Alternatively you can use the I2C API to define a secondary I2C bus on any two broken out GPIO pins.
