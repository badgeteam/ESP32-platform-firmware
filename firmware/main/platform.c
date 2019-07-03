#include "include/platform.h"

#define TAG "platform"

esp_err_t isr_init()
{
	esp_err_t res = gpio_install_isr_service(0);
	if (res == ESP_FAIL) {
		ESP_LOGW(TAG, "Failed to install gpio isr service. Ignoring this.");
		res = ESP_OK;
	}
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "Failed to install gpio isr service: %d", res);
	}
	return res;
}

void platform_init()
{
	if (isr_init() != ESP_OK) restart();
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
