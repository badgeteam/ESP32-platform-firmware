#include <driver_vspi.h>
#include <driver_i2c.h>

#include "include/platform.h"

void platform_init()
{
	INIT_DRIVER(vspi)
	INIT_DRIVER(i2c)
}
