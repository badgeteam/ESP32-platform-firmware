#include "include/platform.h"
#include "include/ota_update.h"
#include "include/factory_reset.h"
#include "driver_rtcmem.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

extern void micropython_entry(void);

esp_err_t nvs_init() {
    const esp_partition_t * nvs_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
    if (nvs_partition == NULL) return ESP_FAIL;        
    esp_err_t res = nvs_flash_init();
    if (res != ESP_OK) {
        res = esp_partition_erase_range(nvs_partition, 0, nvs_partition->size);
        if (res != ESP_OK) return res;
        res = nvs_flash_init();
        if (res != ESP_OK) return res;
    }
    return ESP_OK;
}

int magic() {
	int valueA, valueB;
	if (driver_rtcmem_int_read(0, &valueA) != ESP_OK) return 0;
	if (driver_rtcmem_int_read(1, &valueB) != ESP_OK) return 0;
	if (valueA == (uint8_t)~valueB) {
        return valueA;
    }
	return 0;
}

void reset() {
    fflush(stdout);
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	esp_restart();
}

void app_main() {
    // Start the non-volatile storage component
	if (nvs_init() != ESP_OK) {
        printf("Unable to access the non-volatile storage partition in flash\n");
        printf("This could be caused by an unstable 3.3v supply rail to the ESP32 or by a damaged flash chip.\n");
        reset();
    }

    // Start the other components
	platform_init();

    // Start the application
	switch(magic()) {
		case MAGIC_OTA:
			badge_ota_update();
			break;
		case MAGIC_FACTORY_RESET:
			factory_reset();
			break;
		default:
			micropython_entry();
	}
}
