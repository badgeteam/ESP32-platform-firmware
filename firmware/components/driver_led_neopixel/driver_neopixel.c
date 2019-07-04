#include <sdkconfig.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/spi_master.h>
#include <driver/gpio.h>

#include <driver_vspi.h>
#include <driver_mpr121.h>

#ifdef CONFIG_DRIVER_NEOPIXEL_ENABLE

static const char *TAG = "neopixel";

static spi_device_handle_t driver_leds_spi = NULL;

static esp_err_t driver_neopixel_release_spi(void); // forward declaration

static esp_err_t driver_neopixel_claim_spi(void)
{
	// already claimed?
	if (driver_leds_spi != NULL) return ESP_OK;

	ESP_LOGI(TAG, "claiming VSPI bus");
	driver_vspi_release_and_claim(driver_neopixel_release_spi);

	// (re)initialize leds SPI
	static const spi_bus_config_t buscfg = {
		.mosi_io_num   = CONFIG_NEOPIXEL_PIN,
		.miso_io_num   = -1,  // -1 = unused
		.sclk_io_num   = -1,  // -1 = unused
		.quadwp_io_num = -1,  // -1 = unused
		.quadhd_io_num = -1,  // -1 = unused
	};
	esp_err_t res = spi_bus_initialize(VSPI_HOST, &buscfg, 1);
	if (res != ESP_OK)
		return res;

	static const spi_device_interface_config_t devcfg = {
		.clock_speed_hz = 3200000, // 3.2 Mhz
		.mode           = 0,
		.spics_io_num   = -1,
		.queue_size     = 1,
	};
	res = spi_bus_add_device(VSPI_HOST, &devcfg, &driver_leds_spi);
	if (res != ESP_OK)
		return res;

	ESP_LOGI(TAG, "claiming VSPI bus: done");
	return ESP_OK;
}

static esp_err_t driver_neopixel_release_spi(void)
{
	ESP_LOGI(TAG, "releasing VSPI bus");
	esp_err_t res = spi_bus_remove_device(driver_leds_spi);
	if (res != ESP_OK)
		return res;

	driver_leds_spi = NULL;

	res = spi_bus_free(VSPI_HOST);
	if (res != ESP_OK)
		return res;

	driver_vspi_freed();

	ESP_LOGI(TAG, "releasing VSPI bus: done");
	return ESP_OK;
}

static bool driver_neopixel_active = false;

esp_err_t driver_neopixel_enable(void)
{
	// return if we are already enabled and initialized
	if (driver_neopixel_active)
		return ESP_OK;

	/*esp_err_t res = badge_power_leds_enable();
	if (res != ESP_OK)
		return res;*/

	esp_err_t res = driver_neopixel_claim_spi();
	if (res != ESP_OK)
		return res;

	driver_neopixel_active = true;

	return ESP_OK;
}

esp_err_t driver_neopixel_disable(void)
{
	// return if we are not enabled
	if (!driver_neopixel_active)
		return ESP_OK;

	if (driver_leds_spi != NULL) {
		esp_err_t res = driver_vspi_release_and_claim(NULL);
		if (res != ESP_OK)
			return res;
	}

	driver_neopixel_active = false;

	// configure CONFIG_NEOPIXEL_PIN as high-impedance
	gpio_config_t io_conf = {
		.intr_type    = GPIO_INTR_DISABLE,
		.mode         = GPIO_MODE_INPUT,
		.pin_bit_mask = 1LL << CONFIG_NEOPIXEL_PIN,
		.pull_down_en = 0,
		.pull_up_en   = 0,
	};
	esp_err_t res = gpio_config(&io_conf);
	if (res != ESP_OK)
		return res;

	/*res = badge_power_leds_disable();
	if (res != ESP_OK)
		return res;*/

	return ESP_OK;
}

uint8_t *driver_neopixel_buf = NULL;
int driver_neopixel_buf_len = 0;

esp_err_t driver_neopixel_send_data(uint8_t *data, int len)
{
	static const uint8_t conv[4] = { 0x11, 0x13, 0x31, 0x33 };

	if (driver_neopixel_buf_len < len * 4 + 3)
	{
		if (driver_neopixel_buf != NULL)
			free(driver_neopixel_buf);
		driver_neopixel_buf_len = 0;
		driver_neopixel_buf = malloc(len * 4 + 3);
		if (driver_neopixel_buf == NULL)
			return ESP_ERR_NO_MEM;
	}

	esp_err_t res = driver_neopixel_enable();
	if (res != 0)
		return res;

	// 3 * 24 us 'reset'
	int pos=0;
	driver_neopixel_buf[pos++] = 0;
	driver_neopixel_buf[pos++] = 0;
	driver_neopixel_buf[pos++] = 0;

	int i;
	for (i=0; i<len; i++)
	{
		int v = data[i];

#ifdef CONFIG_NEOPIXEL_TYPE_RGB
		// the WS2812 doesn't have a white led; evenly distribute over other leds.
		if (i < 6*4) // only do conversion for the internal leds
		{
			if ((i|3) >= len)
				break; // not enough data; skip led
			if ((i & 3) == 3)
				continue; // skip the white pixel
			int w = data[i|3];
			v += (w >> 1);
			if (v > 255)
				v = 255;
		}
#endif // CONFIG_NEOPIXEL_TYPE_RGB

		driver_neopixel_buf[pos++] = conv[(v>>6)&3];
		driver_neopixel_buf[pos++] = conv[(v>>4)&3];
		driver_neopixel_buf[pos++] = conv[(v>>2)&3];
		driver_neopixel_buf[pos++] = conv[(v>>0)&3];
	}

	spi_transaction_t t;
	memset(&t, 0, sizeof(t));
	t.length = pos*8;
	t.tx_buffer = driver_neopixel_buf;

	res = driver_neopixel_claim_spi();
	if (res != ESP_OK)
		return res;

	res = spi_device_transmit(driver_leds_spi, &t);
	if (res != ESP_OK)
		return res;

	return ESP_OK;
}

esp_err_t driver_neopixel_init(void)
{
	static bool driver_neopixel_init_done = false;

	if (driver_neopixel_init_done)
		return ESP_OK;

	ESP_LOGD(TAG, "init called");

	// initialize VSPI sharing
	driver_vspi_init();

	// depending on badge_power
	/*esp_err_t res = badge_power_init();
	if (res != ESP_OK)
		return res;*/

	// configure CONFIG_NEOPIXEL_PIN as high-impedance
	gpio_config_t io_conf = {
		.intr_type    = GPIO_INTR_DISABLE,
		.mode         = GPIO_MODE_INPUT,
		.pin_bit_mask = 1LL << CONFIG_NEOPIXEL_PIN,
		.pull_down_en = 0,
		.pull_up_en   = 0,
	};
	esp_err_t res = gpio_config(&io_conf);
	if (res != ESP_OK)
		return res;

	driver_neopixel_init_done = true;

	ESP_LOGD(TAG, "init done");

	return ESP_OK;
}

#else
esp_err_t driver_neopixel_init(void) { return ESP_OK; } //Dummy function.
#endif // CONFIG_DRIVER_NEOPIXEL_ENABLE
