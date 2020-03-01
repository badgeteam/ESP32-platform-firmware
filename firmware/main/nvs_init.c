#include "include/nvs_init.h"

#define TAG "NVS init"

esp_err_t nvs_format()
{
	const esp_partition_t * nvs_partition = esp_partition_find_first(
			ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
	if (nvs_partition == NULL) {
		printf("Could not find the NVS partition, check your partition table!\n");
		restart();
	}
	esp_err_t res = esp_partition_erase_range(nvs_partition, 0, nvs_partition->size);
	//res = spi_flash_erase_sector(nvs_partition->address / SPI_FLASH_SEC_SIZE);
	return res;
}

bool nvs_check_empty()
{
	const esp_partition_t * nvs_partition = esp_partition_find_first(
			ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
	if (nvs_partition == NULL) {
		printf("Could not find the NVS partition, check your partition table!\n");
		restart();
	}
	uint8_t buf[64];
	int res = spi_flash_read(nvs_partition->address, buf, sizeof(buf));
	if (res != ESP_OK) return res;
	static const uint8_t empty[32] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
	if (memcmp(buf, empty, 32) == 0) return true;
	return false;
}

bool nvs_init()
{
	bool was_empty = nvs_check_empty();
	
	esp_err_t res = nvs_flash_init();
	if (res != ESP_OK)
	{
		printf("Formatting NVS partition...\n");
		res = nvs_format();
		if (res != ESP_OK) {
			ESP_LOGE(TAG, "failed to erase NVS partition: %d", res);
			restart();
		}
		res = nvs_flash_init();
		if (res != ESP_OK)
		{
			ESP_LOGE(TAG, "failed to intialize NVS: %d", res);
			restart();
		}
	}
	
	return was_empty;
}
