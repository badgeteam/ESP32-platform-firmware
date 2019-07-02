#include "include/platform.h"

void platform_init()
{
	INIT_DRIVER(vspi)
	INIT_DRIVER(i2c)
	INIT_DRIVER(hub75)
}

bool platform_test()
{
	TEST_DRIVER(vspi)
	TEST_DRIVER(i2c)
	//TEST_DRIVER(hub75)
	return true; //Platform test successful!
}
