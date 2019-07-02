#include "include/platform.h"

void platform_init()
{
	INIT_DRIVER(vspi)
	INIT_DRIVER(i2c)
	INIT_DRIVER(hub75)
}
