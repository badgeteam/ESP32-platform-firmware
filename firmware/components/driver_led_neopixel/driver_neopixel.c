//This driver uses the ESP32 RMT peripheral to drive "Neopixel" compatible LEDs
//The usage of the RMT peripheral has been implemented using work by JSchaenzie:
//you can find his work at https://github.com/JSchaenzle/ESP32-NeoPixel-WS2812-RMT

#include <sdkconfig.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_log.h>

#include <driver/gpio.h>
#include <driver/rmt.h>

#include "driver_mpr121.h"

#ifdef CONFIG_DRIVER_NEOPIXEL_ENABLE

static const char *TAG = "neopixel";

#define CONFIG_DRIVER_NEOPIXEL_RMT_CHANNEL RMT_CHANNEL_0

#define T0H 14  // 0 bit high time
#define T1H 52  // 1 bit high time
#define TL  52  // low time for either bit

static bool  driver_neopixel_active  = false;
rmt_item32_t *driver_neopixel_buf    = NULL;
int          driver_neopixel_buf_len = 0;

esp_err_t driver_neopixel_disable(void)
{
	if (!driver_neopixel_active) return ESP_OK;
	esp_err_t res = rmt_driver_uninstall(CONFIG_DRIVER_NEOPIXEL_RMT_CHANNEL);
	if (res != ESP_OK) return res;
	gpio_config_t io_conf = {
		.intr_type    = GPIO_INTR_DISABLE,
		.mode         = GPIO_MODE_INPUT,
		.pin_bit_mask = 1LL << CONFIG_NEOPIXEL_PIN,
		.pull_down_en = 0,
		.pull_up_en   = 0,
	};
	res = gpio_config(&io_conf);
	if (res != ESP_OK) return res;
	driver_neopixel_active = false;
	return ESP_OK;
}

esp_err_t driver_neopixel_enable(void)
{
	if (driver_neopixel_active) return ESP_OK;
	
	#ifdef CONFIG_DRIVER_NEOPIXEL_MPR121_PIN
		driver_mpr121_set_gpio_level(CONFIG_DRIVER_NEOPIXEL_MPR121_PIN, true); //Enable power
	#endif
	
	rmt_config_t config;
	config.rmt_mode = RMT_MODE_TX;
	config.channel = CONFIG_DRIVER_NEOPIXEL_RMT_CHANNEL;
	config.gpio_num = CONFIG_NEOPIXEL_PIN;
	config.mem_block_num = 3;
	config.tx_config.loop_en = false;
	config.tx_config.carrier_en = false;
	config.tx_config.idle_output_en = true;
	config.tx_config.idle_level = 0;
	config.clk_div = 2;
	esp_err_t res = rmt_config(&config);
	if (res != ESP_OK) return res;
	res = rmt_driver_install(config.channel, 0, 0);
	if (res != ESP_OK) return res;
	driver_neopixel_active = true;
	return ESP_OK;
}

esp_err_t driver_neopixel_prepare_data(uint8_t *data, int len)
{
	if (driver_neopixel_buf != NULL) return ESP_FAIL;
	driver_neopixel_buf = calloc(len * 8, sizeof(rmt_item32_t));
	if (driver_neopixel_buf == NULL) return ESP_FAIL;
	driver_neopixel_buf_len = len * 8;
	for (uint32_t pos = 0; pos < len; pos++) {
		uint32_t mask = 1 << 7;
		for (uint8_t i = 0; i < 8; i++) {
			bool bit = data[pos] & mask;
			driver_neopixel_buf[pos*8 + i] = bit ?
					(rmt_item32_t){{{T1H, 1, TL, 0}}} :
					(rmt_item32_t){{{T0H, 1, TL, 0}}};
			mask >>= 1;
		}
	}
	return ESP_OK;
}

esp_err_t driver_neopixel_free_data()
{
	if (!driver_neopixel_buf) return ESP_FAIL;
	free(driver_neopixel_buf);
	driver_neopixel_buf = NULL;
	driver_neopixel_buf_len = 0;
	return ESP_OK;
}

esp_err_t driver_neopixel_send_data(uint8_t *data, int len)
{
	if (!driver_neopixel_active) { //return ESP_FAIL;
		esp_err_t res = driver_neopixel_enable(); //For backwards compatbibility: enable if not enabled already
		if (res != ESP_OK) return res;
	}
	esp_err_t res = driver_neopixel_prepare_data(data, len);
	if (res != ESP_OK) return res;
	res = rmt_write_items(CONFIG_DRIVER_NEOPIXEL_RMT_CHANNEL, driver_neopixel_buf, driver_neopixel_buf_len, false);
	if (res != ESP_OK) {
		driver_neopixel_free_data();
		return res;
	}
	res = rmt_wait_tx_done(CONFIG_DRIVER_NEOPIXEL_RMT_CHANNEL, portMAX_DELAY); //to-do: make this async?
	if (res != ESP_OK) {
		driver_neopixel_free_data();
		return res;
	}
	res = driver_neopixel_free_data();
	return res;
}

esp_err_t driver_neopixel_init(void)
{
	static bool driver_neopixel_init_done = false;
	if (driver_neopixel_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	//Nothing here.
	
	driver_neopixel_init_done = true;

	ESP_LOGD(TAG, "init done");

	return ESP_OK;
}

#else
esp_err_t driver_neopixel_init(void) { return ESP_OK; } //Dummy function.
#endif // CONFIG_DRIVER_NEOPIXEL_ENABLE
