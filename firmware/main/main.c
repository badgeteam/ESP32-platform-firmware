#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <driver_i2c.h>
#include "ota_update.h"

extern void micropython_entry(void);
extern int esp_rtcmem_read(uint32_t location);

void restart()
{
	for (int i = 10; i >= 0; i--) {
		printf("Restarting in %d seconds...\n", i);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	printf("Restarting now.\n");
	fflush(stdout);
	esp_restart();
}

void logo()
{
	printf("\r\n\r\n\r\n");
	printf("\x1b[31;1mB\x1b[32;1mA\x1b[33;1mD\x1b[34;1mG\x1b[35;1mE\x1b[36;1m.T\x1b[31;1mE\x1b[32;1mA\x1b[33;1mM\x1b[0m FIRMWARE\r\n");
	
#ifdef CONFIG_SHOW_CHIP_INFO
	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);
	printf("\nRunning on an Espressif ESP32 (rev. %d) with %d CPU cores, WiFi%s%s and %dMB %s flash.\r\n",
			chip_info.revision,
			chip_info.cores,
			(chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
			(chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
			spi_flash_get_chip_size() / (1024 * 1024),
			(chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external"
  		);
#endif
	fflush(stdout);
}

void platform_init()
{
#ifdef CONFIG_DRIVER_I2C_ENABLE
	if (driver_i2c_init() != ESP_OK) {
		printf("Could not start the I2C driver!\r\n");
		restart();
	}
#endif
}

int getMagic()
{
	uint8_t magic = esp_rtcmem_read(0);
	uint8_t inv_magic = esp_rtcmem_read(1);
	if (magic == (uint8_t)~inv_magic) {
		return magic;
	}
	return 0;
}

void app_main()
{
	platform_init();
	logo();
	int magic = getMagic();
	switch(magic) {
		case 1:
			badge_ota_update();
			break;
		default:
			micropython_entry();
	}
}
