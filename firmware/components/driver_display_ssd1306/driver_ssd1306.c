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
#include "include/driver_ssd1306.h"

#ifdef CONFIG_DRIVER_SSD1306_ENABLE

static const char *TAG = "ssd1306";

static inline esp_err_t i2c_command(uint8_t value)
{
	esp_err_t res = driver_i2c_write_reg(CONFIG_I2C_ADDR_SSD1306, 0x00, value);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write command(0x%02x): error %d", value, res);
		return res;
	}
	return res;
}

static inline esp_err_t i2c_data(const uint8_t* buffer, uint16_t len)
{
	esp_err_t res = driver_i2c_write_buffer_reg(CONFIG_I2C_ADDR_SSD1306, 0x40, buffer, len);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write data: error %d", res);
		return res;
	}
	return res;
}

esp_err_t driver_ssd1306_reset(void)
{
#if CONFIG_PIN_NUM_SSD1306_RESET >= 0
	gpio_set_level(CONFIG_PIN_NUM_SSD1306_RESET, false);
	vTaskDelay(10 / portTICK_PERIOD_MS);
	gpio_set_level(CONFIG_PIN_NUM_SSD1306_RESET, true);
	vTaskDelay(5 / portTICK_PERIOD_MS);
#endif
	return ESP_OK;
}

esp_err_t driver_ssd1306_init(void)
{
	static bool driver_ssd1306_init_done = false;
	if (driver_ssd1306_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	esp_err_t res = driver_i2c_init();
	if (res != ESP_OK) return res;
#if CONFIG_PIN_NUM_SSD1306_RESET >= 0
	gpio_set_direction(CONFIG_PIN_NUM_SSD1306_RESET, GPIO_MODE_OUTPUT);
	driver_ssd1306_reset();
#endif
	
#ifdef CONFIG_SSD1306_12832
	res=i2c_command(0xae); // SSD1306_DISPLAYOFF
	if (res != ESP_OK) return res;
	res=i2c_command(0xd5); // SSD1306_SETDISPLAYCLOCKDIV
	if (res != ESP_OK) return res;
	i2c_command(0xf0); // Sets frequency to highest value and divider to 1 for less flicker
	if (res != ESP_OK) return res;
	i2c_command(0xa8); // SSD1306_SETMULTIPLEX
	if (res != ESP_OK) return res;
	i2c_command(0x1f); // 1/32
	if (res != ESP_OK) return res;
	i2c_command(0xd3); // SSD1306_SETDISPLAYOFFSET
	if (res != ESP_OK) return res;
	i2c_command(0x00); // 0 no offset
	if (res != ESP_OK) return res;
	i2c_command(0x40); // SSD1306_SETSTARTLINE line #0
	if (res != ESP_OK) return res;
	i2c_command(0x8d); // SSD1306_CHARGEPUMP
	if (res != ESP_OK) return res;
	i2c_command(0x14); // Charge pump on
	if (res != ESP_OK) return res;
	i2c_command(0x20); // SSD1306_MEMORYMODE
	if (res != ESP_OK) return res;
	i2c_command(0x00); // 0x0 act like ks0108
	if (res != ESP_OK) return res;
	i2c_command(0xa1); // SSD1306_SEGREMAP | 1
	if (res != ESP_OK) return res;
	i2c_command(0xc8); // SSD1306_COMSCANDEC
	if (res != ESP_OK) return res;
	i2c_command(0xda); // SSD1306_SETCOMPINS
	if (res != ESP_OK) return res;
	i2c_command(0x02);
	if (res != ESP_OK) return res;
	i2c_command(0x81); // SSD1306_SETCONTRAST
	if (res != ESP_OK) return res;
	i2c_command(0x2f);
	if (res != ESP_OK) return res;
	i2c_command(0xd9); // SSD1306_SETPRECHARGE
	if (res != ESP_OK) return res;
	i2c_command(0xf1);
	if (res != ESP_OK) return res;
	i2c_command(0xdb); // SSD1306_SETVCOMDETECT
	if (res != ESP_OK) return res;
	i2c_command(0x40);
	if (res != ESP_OK) return res;
	i2c_command(0x2e); // SSD1306_DEACTIVATE_SCROLL
	if (res != ESP_OK) return res;
	i2c_command(0xa4); // SSD1306_DISPLAYALLON_RESUME
	if (res != ESP_OK) return res;
	i2c_command(0xa6); // SSD1306_NORMALDISPLAY
	if (res != ESP_OK) return res;
#else
	i2c_command(0xae); // SSD1306_DISPLAYOFF
	if (res != ESP_OK) return res;
	i2c_command(0xd5); // SSD1306_SETDISPLAYCLOCKDIV
	if (res != ESP_OK) return res;
	i2c_command(0x80); // Suggested value 0x80
	if (res != ESP_OK) return res;
	i2c_command(0xa8); // SSD1306_SETMULTIPLEX
	if (res != ESP_OK) return res;
	i2c_command(0x3f); // 1/64
	if (res != ESP_OK) return res;
	i2c_command(0xd3); // SSD1306_SETDISPLAYOFFSET
	if (res != ESP_OK) return res;
	i2c_command(0x00); // 0 no offset
	if (res != ESP_OK) return res;
	i2c_command(0x40); // SSD1306_SETSTARTLINE line #0
	if (res != ESP_OK) return res;
	i2c_command(0x20); // SSD1306_MEMORYMODE
	if (res != ESP_OK) return res;
	i2c_command(0x01); // 0x0 act like ks0108 / 0x01 vertical addressing mode
	if (res != ESP_OK) return res;
	i2c_command(0xa1); // SSD1306_SEGREMAP | 1
	if (res != ESP_OK) return res;
	i2c_command(0xc8); // SSD1306_COMSCANDEC
	if (res != ESP_OK) return res;
	i2c_command(0xda); // SSD1306_SETCOMPINS
	if (res != ESP_OK) return res;
	i2c_command(0x12);
	if (res != ESP_OK) return res;
	i2c_command(0x81); // SSD1306_SETCONTRAST
	if (res != ESP_OK) return res;
	i2c_command(0xcf);
	if (res != ESP_OK) return res;
	i2c_command(0xd9); // SSD1306_SETPRECHARGE
	if (res != ESP_OK) return res;
	i2c_command(0xf1);
	if (res != ESP_OK) return res;
	i2c_command(0xdb); // SSD1306_SETVCOMDETECT
	if (res != ESP_OK) return res;
	i2c_command(0x30);
	if (res != ESP_OK) return res;
	i2c_command(0x8d); // SSD1306_CHARGEPUMP
	if (res != ESP_OK) return res;
	i2c_command(0x14); // Charge pump on
	if (res != ESP_OK) return res;
	i2c_command(0x2e); // SSD1306_DEACTIVATE_SCROLL
	if (res != ESP_OK) return res;
	i2c_command(0xa4); // SSD1306_DISPLAYALLON_RESUME
	if (res != ESP_OK) return res;
	i2c_command(0xa6); // SSD1306_NORMALDISPLAY
	if (res != ESP_OK) return res;
#endif
	i2c_command(0xaf); // SSD1306_DISPLAYON
	if (res != ESP_OK) return res;
	
	uint8_t buffer[1024] = {0};
	res = driver_ssd1306_write(buffer); //Clear screen
	if (res != ESP_OK) return res;
	
	driver_ssd1306_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

esp_err_t driver_ssd1306_write_part(const uint8_t *buffer, int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
	uint16_t addr0 = x0*(SSD1306_HEIGHT/8);
	uint16_t addr1 = x1*(SSD1306_HEIGHT/8) + (SSD1306_HEIGHT/8);
	uint16_t length = addr1-addr0;
	
	esp_err_t res;
	res = i2c_command(0x21); //Column address
	if (res != ESP_OK) return res;
	res = i2c_command(x0); //Column start
	if (res != ESP_OK) return res;
	res = i2c_command(x1);//SSD1306_WIDTH-1); //Column end
	if (res != ESP_OK) return res;
	res = i2c_command(0x22); //Page address
	if (res != ESP_OK) return res;
	res = i2c_command(0); //Page start
	if (res != ESP_OK) return res;
	res = i2c_command(7);   //Page end
	if (res != ESP_OK) return res;
	res = i2c_data(buffer+addr0, length);
	if ( res != ESP_OK) return res;
	return res;
}

esp_err_t driver_ssd1306_write(const uint8_t *buffer)
{
	esp_err_t res;
	res = i2c_command(0x21); //Column address
	if (res != ESP_OK) return res;
	res = i2c_command(   0); //Column start
	if (res != ESP_OK) return res;
	res = i2c_command( SSD1306_WIDTH-1); //Column end
	if (res != ESP_OK) return res;
	
	res = i2c_command(0x22); //Page address
	if (res != ESP_OK) return res;
	res = i2c_command(0); //Page start
	if (res != ESP_OK) return res;
	res = i2c_command(7);   //Page end
	if (res != ESP_OK) return res;
	
	res = i2c_data(buffer, SSD1306_BUFFER_SIZE);
	if ( res != ESP_OK) return res;

	ESP_LOGD(TAG, "i2c write data ok");
	return res;
}
#else
esp_err_t driver_ssd1306_init(void) { return ESP_OK; } //Dummy function.
#endif // CONFIG_DRIVER_SSD1306_ENABLE
