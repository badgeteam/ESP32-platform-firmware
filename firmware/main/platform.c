#include "include/platform.h"

void platform_init()
{
	INIT_DRIVER(vspi)
	INIT_DRIVER(i2c)
	INIT_DRIVER(hub75)
	INIT_DRIVER(mpr121)
}

void platform_first_boot()
{
	config_set_u8("system", "selftest", 0); //We've not yet passed the self-test
	printf("\n\nThis is the first boot: running platform self-test...\n\n");
	TEST_DRIVER(vspi)
	TEST_DRIVER(i2c)
	//TEST_DRIVER(hub75)
	config_set_u8("system", "selftest", 1);
	printf("\n\nAll self-test functions have been executed.\n\n");
}
