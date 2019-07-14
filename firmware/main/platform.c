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
	INIT_DRIVER(vspi)         //Generic VSPI bus driver
	INIT_DRIVER(i2c)          //Generic I2C bus driver
	INIT_DRIVER(hub75)        //LED matrix as found on the CampZone 2019 badge
	INIT_DRIVER(erc12864)     //128x64 LCD screen as found on the Disobey 2019 badge
	INIT_DRIVER(ssd1306)      //128x64 OLED screen as found on the Disobey 2020 badge
	INIT_DRIVER(eink)         //296x128 e-ink display as found on the SHA2017 and HackerHotel 2019 badges
	INIT_DRIVER(framebuffer)  //Framebuffer driver with basic drawing routines
	INIT_DRIVER(mpr121)       //I/O expander with touch inputs as found on the SHA2017 and HackerHotel 2019 badges
	INIT_DRIVER(disobey_samd) //I/O via the SAMD co-processor on the Disobey 2019 badge
	INIT_DRIVER(neopixel)     //Addressable LEDs as found on the SHA2017 and HackerHotel 2019 badges
}
