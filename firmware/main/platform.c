#include "include/platform.h"

void platform_init()
{
	INIT_DRIVER(vspi)
	INIT_DRIVER(i2c)
	INIT_DRIVER(hub75)
}

void platform_first_boot()
{
	printf("\n\n--- FIRST BOOT, RUNNING DRIVER SETUP AND TEST FUNCTIONS ---\n\n");
	TEST_DRIVER(vspi)
	TEST_DRIVER(i2c)
	//TEST_DRIVER(hub75)
	printf("\n\n--- DONE ---\n\n");
}
