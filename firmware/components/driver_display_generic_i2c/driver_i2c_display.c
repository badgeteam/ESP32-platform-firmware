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
#include "include/driver_i2c_display.h"

#ifdef CONFIG_DRIVER_DISPLAY_I2C_ENABLE

#define TAG "display_i2c"

esp_err_t driver_i2c_display_init(void)
{
	static bool driver_display_generic_i2c_done = false;
	if (driver_display_generic_i2c_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	esp_err_t res = driver_i2c_init();
	if (res != ESP_OK) return res;

	// Dummy write to "wake up" i2c bus. Needed because the first operation always times out for some reason.
	res = driver_i2c_write_reg(CONFIG_DISPLAY_I2C_ADDR, CONFIG_DISPLAY_I2C_FRAMEBUF_REG, 0);
	if (res != ESP_OK) {
		ESP_LOGW(TAG, "display i2c init: dummy write failed %d", res);
	}

	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

esp_err_t driver_i2c_display_write(const uint8_t *buffer)
{
	esp_err_t res = driver_i2c_write_reg_n(CONFIG_DISPLAY_I2C_ADDR, CONFIG_DISPLAY_I2C_FRAMEBUF_REG,
	                                        buffer, I2C_DISPLAY_BUFFER_SIZE);

	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c framebuffer write error %d", res);
		return res;
	}

	res = driver_i2c_write_reg(CONFIG_DISPLAY_I2C_ADDR, CONFIG_DISPLAY_I2C_DIRTYBYTE_REG, 1);

	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c framebuffer write error %d", res);
		return res;
	}

	ESP_LOGD(TAG, "i2c write data ok");
	return res;
}
#else
esp_err_t driver_i2c_display_init(void) { return ESP_OK; } //Dummy function.
#endif // CONFIG_DRIVER_DISPLAY_I2C_ENABLE
