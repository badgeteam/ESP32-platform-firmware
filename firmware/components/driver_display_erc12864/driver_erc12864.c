#include <sdkconfig.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>

#include <buses.h>
#include "include/driver_erc12864.h"

#ifdef CONFIG_DRIVER_ERC12864_ENABLE

static const char *TAG = "erc12864";

static inline esp_err_t set_page_address(uint8_t page)
{
	esp_err_t res = driver_i2c_write_byte(CONFIG_DRIVER_ERC12864_I2C_BUS, CONFIG_I2C_ADDR_ERC12864, 0xb0 | page);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write page(0x%02x): error %d", page, res);
		return res;
	}
	return res;
}

static inline esp_err_t set_column(uint8_t column)
{
	uint8_t buffer[] = {0x10 | (column>>4), (0x0f&column) | 0x04};
	esp_err_t res = driver_i2c_write_buffer(CONFIG_DRIVER_ERC12864_I2C_BUS, CONFIG_I2C_ADDR_ERC12864, buffer, 2);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write column(0x%02x): error %d", column, res);
		return res;
	}
	return res;
}

esp_err_t driver_erc12864_set_contrast(uint8_t contrast)
{
	if (contrast > 63) contrast = 63;
	uint8_t buffer[] = {0x81, contrast};
	esp_err_t res = driver_i2c_write_buffer(CONFIG_DRIVER_ERC12864_I2C_BUS, CONFIG_I2C_ADDR_ERC12864, buffer, 2);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write contrast error %d", res);
		return res;
	}
	return res;
}

esp_err_t driver_erc12864_init(void)
{
	esp_err_t res = driver_i2c_write_byte(CONFIG_DRIVER_ERC12864_I2C_BUS, CONFIG_I2C_ADDR_ERC12864, 0x2f);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write init: error %d", res);
		return res;
	}
	vTaskDelay(10 / portTICK_PERIOD_MS);
	driver_i2c_write_byte(CONFIG_DRIVER_ERC12864_I2C_BUS, CONFIG_I2C_ADDR_ERC12864, 0xa2);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write init: error %d", res);
		return res;
	}
	vTaskDelay(10 / portTICK_PERIOD_MS);
	driver_i2c_write_byte(CONFIG_DRIVER_ERC12864_I2C_BUS, CONFIG_I2C_ADDR_ERC12864, 0xa1);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write init: error %d", res);
		return res;
	}
	vTaskDelay(10 / portTICK_PERIOD_MS);
	driver_i2c_write_byte(CONFIG_DRIVER_ERC12864_I2C_BUS, CONFIG_I2C_ADDR_ERC12864, 0xc8);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write init: error %d", res);
		return res;
	}
	vTaskDelay(10 / portTICK_PERIOD_MS);
	driver_i2c_write_byte(CONFIG_DRIVER_ERC12864_I2C_BUS, CONFIG_I2C_ADDR_ERC12864, 0xa7);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write init: error %d", res);
		return res;
	}
	vTaskDelay(10 / portTICK_PERIOD_MS);
	driver_i2c_write_byte(CONFIG_DRIVER_ERC12864_I2C_BUS, CONFIG_I2C_ADDR_ERC12864, 0x24);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write init: error %d", res);
		return res;
	}
	vTaskDelay(10 / portTICK_PERIOD_MS);
	driver_erc12864_set_contrast(37);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write init: error %d", res);
		return res;
	}
	vTaskDelay(10 / portTICK_PERIOD_MS);
	driver_i2c_write_byte(CONFIG_DRIVER_ERC12864_I2C_BUS, CONFIG_I2C_ADDR_ERC12864, 0x40);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write init: error %d", res);
		return res;
	}
	vTaskDelay(10 / portTICK_PERIOD_MS);
	driver_i2c_write_byte(CONFIG_DRIVER_ERC12864_I2C_BUS, CONFIG_I2C_ADDR_ERC12864, 0xaf);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write init: error %d", res);
		return res;
	}
	vTaskDelay(10 / portTICK_PERIOD_MS);
	return ESP_OK;
}

unsigned char reverse(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

esp_err_t driver_erc12864_write(const uint8_t *buffer)
{
	esp_err_t res;

	for (uint8_t page = 0; page < 8; page++) {
		res = set_page_address(page);
		if (res != ESP_OK) break;
		for (uint8_t num = 0; num < 4; num++) {
			res = set_column(num*0x20);
			if (res != ESP_OK) break;
			uint16_t offset = 128*page + 32*num;
			res = driver_i2c_write_buffer(CONFIG_I2C_ADDR_ERC12864+1, buffer+offset, 32);
			if (res != ESP_OK) break;
		}
	}

	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write data error %d", res);
		return res;
	}

	ESP_LOGD(TAG, "i2c write data ok");
	return res;
}

esp_err_t driver_erc12864_write_part(const uint8_t *buffer, int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
	esp_err_t res = ESP_OK;
	
	uint8_t startPage = y0/8;
	uint8_t endPage = y1/8;
	uint8_t startColumn = x0/32;
	uint8_t endColumn = x1/32;
	
	//printf("[DR] area (%u, %u), end (%u, %u)\n", startColumn, startPage, endColumn, endPage);
	
	for (uint8_t page = startPage; page <= endPage; page++) {
		res = set_page_address(page);
		if (res != ESP_OK) break;
		for (uint8_t num = startColumn; num <= endColumn; num++) {
			res = set_column(num*0x20);
			if (res != ESP_OK) break;
			uint16_t offset = 128*page + 32*num;
			res = driver_i2c_write_buffer(CONFIG_I2C_ADDR_ERC12864+1, buffer+offset, 32);
			if (res != ESP_OK) break;
		}
	}

	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write data error %d", res);
		return res;
	}

	ESP_LOGD(TAG, "i2c write data ok");
	return res;
}
#else
esp_err_t driver_erc12864_init(void) { return ESP_OK; } //Dummy function.
#endif // CONFIG_DRIVER_ERC12864_ENABLE
