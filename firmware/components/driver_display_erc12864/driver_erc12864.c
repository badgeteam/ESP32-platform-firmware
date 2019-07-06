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

#include <driver_i2c.h>
#include "include/driver_erc12864.h"

#ifdef CONFIG_DRIVER_ERC12864_ENABLE

static const char *TAG = "erc12864";

static inline esp_err_t set_page_address(uint8_t page)
{
	esp_err_t res = driver_i2c_write_reg(CONFIG_I2C_ADDR_ERC12864, 0x38, 0xb0 | page);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write page(0x%02x): error %d", page, res);
		return res;
	}
	return res;
}

static inline esp_err_t set_column(uint8_t column)
{
	uint8_t buffer[] = {0x38, 0x10 | (column>>4), (0x0f&column) | 0x04};
	esp_err_t res = driver_i2c_write_buffer(CONFIG_I2C_ADDR_ERC12864, buffer, 3);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write column(0x%02x): error %d", column, res);
		return res;
	}
	return res;
}

esp_err_t driver_erc12864_init(void)
{
	static bool driver_erc12864_init_done = false;
	if (driver_erc12864_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	esp_err_t res = driver_i2c_init();
	if (res != ESP_OK) return res;
	
	uint8_t initSeq[] = {0x38, 0x2f, 0xA2, 0xA1, 0xC8, 0x24, 0x81, 37, 0x40, 0xAF};
	res = driver_i2c_write_buffer(CONFIG_I2C_ADDR_ERC12864, initSeq, 10);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write init: error %d", res);
		return res;
	}

	uint8_t buffer[1024] = {0};
	driver_erc12864_write(buffer); //Clear screen

	driver_erc12864_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

unsigned char reverse(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

#ifdef CONFIG_ERC12864_FLIP
	bool flip = true;
#else
	bool flip = false;
#endif
	
void driver_erc12864_set_rotation(bool newFlip)
{
	flip = newFlip;
	ESP_LOGD(TAG, "erc12864 flip: %s", flip ? "enabled" : "disabled");
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
			if (flip) {
				uint8_t data[32] = {0};
				for (uint8_t i = 0; i < 32; i++) {
					data[i] = reverse(buffer[1023-offset-i]);
				}
				res = driver_i2c_write_buffer(CONFIG_I2C_ADDR_ERC12864+1, data, 32);
			} else {
				res = driver_i2c_write_buffer(CONFIG_I2C_ADDR_ERC12864+1, buffer+offset, 32);
			}
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
