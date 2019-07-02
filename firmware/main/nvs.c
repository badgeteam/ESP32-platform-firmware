#include "include/nvs.h"

#define TAG "NVS"

esp_err_t nvs_format(bool wipe)
{
	const esp_partition_t * nvs_partition = esp_partition_find_first(
			ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
	if (nvs_partition == NULL) {
		ESP_LOGE(TAG, "NVS partition not found!");
		return ESP_FAIL;
	}
	if (wipe) {
		esp_err_t res = esp_partition_erase_range(nvs_partition, 0, nvs_partition->size);
		//res = spi_flash_erase_sector(nvs_partition->address / SPI_FLASH_SEC_SIZE);
		if (res != ESP_OK) {
			ESP_LOGE(TAG, "failed to erase NVS partition: %d", res);
			return res;
		}
	}
	printf("Initializing NVS partition...\n");
	return nvs_flash_init();
}

esp_err_t nvs_check()
{
	const esp_partition_t * nvs_partition = esp_partition_find_first(
			ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
	if (nvs_partition == NULL) {
		ESP_LOGE(TAG, "NVS partition not found!");
		return ESP_FAIL;
	}
	uint8_t buf[64];
	int res = spi_flash_read(nvs_partition->address, buf, sizeof(buf));
	if (res != ESP_OK) return res;
	static const uint8_t empty[16] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
	if (memcmp(buf, empty, 16) == 0) return ESP_ERR_NVS_NOT_INITIALIZED;
	return ESP_OK;
}

void nvs_init()
{
	esp_err_t res = nvs_check();
	if (res == ESP_ERR_NVS_NOT_INITIALIZED) {
		//Failed because NVS partition has not been initialized.
		printf("NVS partition seems to be empty!\n");
		nvs_format(false);
	} else if (res != ESP_OK) {
		ESP_LOGE(TAG, "failed to read from NVS partition: %d", res);
		restart();
	}
}
