#include "include/system.h"

#include "driver_rtcmem.h"

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

void halt()
{
	printf("--- HALTED ---\n");
	fflush(stdout);
	while (1) { esp_deep_sleep_start(); }
}

void logo()
{
	printf("\r\n\r\n\r\n");
	printf("\x1b[33;1mBADGE\x1b[34;1m.TEAM\x1b[0m\r\n");
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

int get_magic()
{
	int magic, inv_magic;
	if (driver_rtcmem_int_read(0, &magic)     != ESP_OK) return 0;
	if (driver_rtcmem_int_read(1, &inv_magic) != ESP_OK) return 0;
	
	if (magic == (uint8_t)~inv_magic) return magic;
	return 0;
}
