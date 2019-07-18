#include "include/platform.h"
#include "driver_framebuffer.h"

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

bool fbReady = false;

void fatal_error(const char* message)
{
	printf("A fatal error occcured while initializing the driver for '%s'.\n", message);
	if (fbReady) {
		#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE
		#ifdef CONFIG_DRIVER_EINK_ENABLE
			driver_framebuffer_fill(COLOR_WHITE);
			driver_framebuffer_setTextColor(COLOR_BLACK);
			driver_framebuffer_setFont(&freesansbold12pt7b);
			driver_framebuffer_setCursor(0,0);
			driver_framebuffer_setTextScale(1,1);
			driver_framebuffer_print("Fatal error\n");
			driver_framebuffer_setFont(&freesans8pt7b);
			driver_framebuffer_print("Failure while starting driver.\n");
			driver_framebuffer_print(message);
			driver_framebuffer_print("\n\nRestart in 10 seconds...\n");
			driver_framebuffer_flush();
		#endif
		#if defined(CONFIG_DRIVER_SSD1306_ENABLE) || defined(CONFIG_DRIVER_ERC12846_ENABLE)
			driver_framebuffer_fill(COLOR_BLACK);
			driver_framebuffer_setTextColor(COLOR_WHITE);
			driver_framebuffer_setCursor(0,0);
			driver_framebuffer_setTextScale(1,1);
			driver_framebuffer_setFont(&freesan8pt7b);
			driver_framebuffer_print("Fatal error\n");
			driver_framebuffer_print("Driver failed:\n");
			driver_framebuffer_print(message);
			driver_framebuffer_flush();
	#endif
	#endif
	}
	restart();
}

void platform_init()
{
	if (isr_init() != ESP_OK) restart();
	INIT_DRIVER(vspi         , "VSPI BUS"   ) //Generic VSPI bus driver
	INIT_DRIVER(i2c          , "I2C BUS"    ) //Generic I2C bus driver
	INIT_DRIVER(hub75        , "HUB75"      ) //LED matrix as found on the CampZone 2019 badge
	INIT_DRIVER(erc12864     , "ERC12864"   ) //128x64 LCD screen as found on the Disobey 2019 badge
	INIT_DRIVER(ssd1306      , "SSD1306"    ) //128x64 OLED screen as found on the Disobey 2020 badge
	INIT_DRIVER(eink         , "E-INK"      ) //296x128 e-ink display as found on the SHA2017 and HackerHotel 2019 badges
	INIT_DRIVER(ili9341      , "ILI9341"    ) //LCD display on wrover kit
	INIT_DRIVER(framebuffer  , "FRAMEBUFFER") //Framebuffer driver with basic drawing routines
	fbReady = true;                           //Notify the error handler that framebuffer support is now available
	INIT_DRIVER(mpr121       , "MPR121"     ) //I/O expander with touch inputs as found on the SHA2017 and HackerHotel 2019 badges
	INIT_DRIVER(disobey_samd , "SAMD"       ) //I/O via the SAMD co-processor on the Disobey 2019 badge
	INIT_DRIVER(neopixel     , "NEOPIXEL"   ) //Addressable LEDs as found on the SHA2017 and HackerHotel 2019 badges
}
