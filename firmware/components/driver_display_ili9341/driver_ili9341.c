#include <sdkconfig.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <soc/gpio_reg.h>
#include <soc/gpio_sig_map.h>
#include <soc/gpio_struct.h>
#include <soc/spi_reg.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>

#include <driver_vspi.h>
#include "include/driver_ili9341.h"

#ifdef CONFIG_DRIVER_ILI9341_ENABLE

#define ILI9341_MAX_TRANSFERSIZE 320*128*2+32

static const char *TAG = "ili9341";

static spi_device_handle_t spi_bus = NULL;

static esp_err_t driver_ili9341_release_spi(void)
{
	ESP_LOGI(TAG, "releasing VSPI bus");
	esp_err_t res = spi_bus_remove_device(spi_bus);
	assert(res == ESP_OK);
	spi_bus = NULL;
	res = spi_bus_free(VSPI_HOST);
	assert(res == ESP_OK);
	driver_vspi_freed();
	ESP_LOGI(TAG, "releasing VSPI bus: done");
	return ESP_OK;
}

static void driver_ili9341_spi_pre_transfer_callback(spi_transaction_t *t)
{
	uint8_t dc_level = *((uint8_t *) t->user);
	gpio_set_level(CONFIG_PIN_NUM_ILI9341_DCX, (int) dc_level);
}

static esp_err_t driver_ili9341_claim_spi(void)
{
	if (spi_bus != NULL) return ESP_OK; // already claimed?
	ESP_LOGI(TAG, "claiming VSPI bus");
	driver_vspi_release_and_claim(driver_ili9341_release_spi);

	static const spi_bus_config_t buscfg = {
		.mosi_io_num     = CONFIG_PIN_NUM_ILI9341_MOSI,
		.miso_io_num     = CONFIG_PIN_NUM_ILI9341_MISO,
		.sclk_io_num     = CONFIG_PIN_NUM_ILI9341_CLK,
		.quadwp_io_num   = -1,
		.quadhd_io_num   = -1,
		.max_transfer_sz = ILI9341_MAX_TRANSFERSIZE,
	};
	esp_err_t res = spi_bus_initialize(VSPI_HOST, &buscfg, 2);
	if (res != ESP_OK) return res;
	static const spi_device_interface_config_t devcfg = {
		.clock_speed_hz = 40 * 1000 * 1000,
		.mode           = 0,  // SPI mode 0
		.spics_io_num   = CONFIG_PIN_NUM_ILI9341_CS,
		.queue_size     = 1,
		.flags          = SPI_DEVICE_HALFDUPLEX,
		.pre_cb         = driver_ili9341_spi_pre_transfer_callback, // Specify pre-transfer callback to handle D/C line
	};
	res = spi_bus_add_device(VSPI_HOST, &devcfg, &spi_bus);
	if (res != ESP_OK) return res;
	ESP_LOGI(TAG, "claiming VSPI bus: done");
	return res;
}

const uint8_t ili9341_init_data[] = {

    0xEF, 3, 0x03,0x80,0x02,

    0xCF, 3, 0x00,0XC1,0X30,

    0xED, 4, 0x64,0x03,0X12,0X81,

    0xE8, 3, 0x85,0x00,0x78,

    0xCB, 5, 0x39,0x2C,0x00,0x34,0x02,

    0xF7, 1, 0x20,

    0xEA, 2, 0x00,0x00,

    0xC0, 1, 0x23,

    0xC1, 1, 0x10,

    0xC5, 2, 0x3e,0x28,

    0xC7, 1, 0x86,

    0x36, 1, 0x48,

    0x3A, 1, 0x55,

    0xB1, 2, 0x00,0x18,

    0xB6, 3, 0x08,0x82,0x27,

    0xF2, 1, 0x00,

    0x26, 1, 0x01,

    0xE0, 15, 0x0F,0x31,0x2B,0x0C,0x0E,0x08,0x4E,0xF1,0x37,0x07,0x10,0x03,0x0E,0x09,0x00,

    0xE1, 15, 0x00,0x0E,0x14,0x03,0x11,0x07,0x31,0xC1,0x48,0x08,0x0F,0x0C,0x31,0x36,0x0F,

    0x00

};

esp_err_t driver_ili9341_send(const uint8_t *data, int len, const uint8_t dc_level)
{
	if (len == 0) return ESP_OK;
	esp_err_t res = driver_ili9341_claim_spi();
	if (res != ESP_OK) return res;
	spi_transaction_t t = {
		.length = len * 8,  // transaction length is in bits
		.tx_buffer = data,
		.user = (void *) &dc_level,
	};
	res = spi_device_transmit(spi_bus, &t);
	return res;
}

esp_err_t driver_ili9341_receive(uint8_t *data, int len, const uint8_t dc_level)
{
	if (len == 0) return ESP_OK;
	esp_err_t res = driver_ili9341_claim_spi();
	if (res != ESP_OK) return res;
	spi_transaction_t t = {
		.length = len * 8,  // transaction length is in bits
		.rxlength = len * 8,
		.rx_buffer = data,
		.user = (void *) &dc_level,
	};
	res = spi_device_transmit(spi_bus, &t);
	return res;
}

esp_err_t driver_ili9341_write_initData(const uint8_t * data)
{
	uint8_t cmd, len;
	while(true) {
		cmd = *data++;
		if(!cmd) return ESP_OK; //END
		len = *data++;
		driver_ili9341_send(&cmd, 1, false);
		driver_ili9341_send(data, len, true);
		data+=len;
	}
	return ESP_OK;
}

esp_err_t driver_ili9341_send_command(uint8_t cmd)
{
	return driver_ili9341_send(&cmd, 1, false);
}

esp_err_t driver_ili9341_send_u32(uint32_t data)
{
	uint8_t buffer[4];
	buffer[0] = (data>>24)&0xFF;
	buffer[1] = (data>>16)&0xFF;
	buffer[2] = (data>> 8)&0xFF;
	buffer[3] = data      &0xFF;
	return driver_ili9341_send(buffer, 4, true);
}

esp_err_t driver_ili9341_set_addr_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	uint32_t xa = ((uint32_t)x << 16) | (x+w-1);
	uint32_t ya = ((uint32_t)y << 16) | (y+h-1);
	esp_err_t res;
	res = driver_ili9341_send_command(ILI9341_CASET);
	if (res != ESP_OK) return res;
	res = driver_ili9341_send_u32(xa);
	if (res != ESP_OK) return res;
	res = driver_ili9341_send_command(ILI9341_RASET);
	if (res != ESP_OK) return res;
	res = driver_ili9341_send_u32(ya);
	if (res != ESP_OK) return res;
	res = driver_ili9341_send_command(ILI9341_RAMWR);
	return res;
}

esp_err_t driver_ili9341_read_id(uint32_t* result)
{
	uint8_t commands[2] = {0xD9, 0x04};
	driver_ili9341_send(commands, 1, false);
	driver_ili9341_send(commands+1, 1, true);
	uint8_t buffer[3];
	driver_ili9341_receive(buffer, 3, true);
	*result = (buffer[0]<<16)+(buffer[1]<<8)+buffer[2];
	return ESP_OK;
}

esp_err_t driver_ili9341_reset(void) {
	esp_err_t res = gpio_set_level(CONFIG_PIN_NUM_ILI9341_RESET, false);
	if (res != ESP_OK) return res;
	ets_delay_us(200000);
	res = gpio_set_level(CONFIG_PIN_NUM_ILI9341_RESET, true);
	if (res != ESP_OK) return res;
	ets_delay_us(200000);
	return ESP_OK;
}

esp_err_t driver_ili9341_init(void)
{
	static bool driver_ili9341_init_done = false;
	if (driver_ili9341_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	
	gpio_set_direction(5, GPIO_MODE_OUTPUT);
	gpio_set_level(5, false);
	
	esp_err_t res;
	
	res = gpio_set_direction(CONFIG_PIN_NUM_ILI9341_RESET, GPIO_MODE_OUTPUT);
	if (res != ESP_OK) return res;
	res = gpio_set_direction(CONFIG_PIN_NUM_ILI9341_DCX, GPIO_MODE_OUTPUT);
	if (res != ESP_OK) return res;
	res = driver_ili9341_reset();
	if (res != ESP_OK) return res;
	res = driver_ili9341_write_initData(ili9341_init_data);
	
	driver_ili9341_send_command(ILI9341_SLPOUT);
	vTaskDelay(200 / portTICK_PERIOD_MS);
	driver_ili9341_send_command(ILI9341_DISPON);
	vTaskDelay(200 / portTICK_PERIOD_MS);
	if (res != ESP_OK) return res;
	
	driver_ili9341_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

esp_err_t driver_ili9341_write(const uint8_t *buffer)
{
	return driver_ili9341_write_partial(buffer, 0, 0, ILI9341_WIDTH, ILI9341_HEIGHT);
}

esp_err_t driver_ili9341_write_partial_direct(const uint8_t *buffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{ //Without conversion
	if (x0 > x1) return ESP_FAIL;
	if (y0 > y1) return ESP_FAIL;
	uint16_t w = x1-x0;
	uint16_t h = y1-y0;
	esp_err_t res = driver_ili9341_set_addr_window(x0, y0, w, h);
	if (res != ESP_OK) return res;
	res = driver_ili9341_send(buffer, w*h*2, true);
	return res;
}

esp_err_t driver_ili9341_write_partial(const uint8_t *buffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{ //With conversion from framebuffer
	esp_err_t res = ESP_OK;
	if (x0 > x1) {
		printf("X0 %u > X1 %u\n", x0, x1);
		return ESP_FAIL;
	}
	if (y0 > y1) {
		printf("Y0 %u > Y1 %u\n", y0, y1);
		return ESP_FAIL;
	}
	if (x1 >= ILI9341_WIDTH)  x1 = ILI9341_WIDTH-1;
	if (y1 >= ILI9341_HEIGHT) y1 = ILI9341_HEIGHT-1;
	
	uint16_t w = x1-x0+1;
	uint16_t h = y1-y0+1;
	
	if ((w > 300) || (w*h*2 > ILI9341_MAX_TRANSFERSIZE)) { //Just copy whole lines if most of the line needs to be refreshed anyway
		//printf("Refreshing %ux%u area at (%u,%u) (%u,%u) using method 1\n", w, h, x0, y0, x1, y1);
		while (h > 0) {
			uint16_t lines = h;
			if (lines > 120) lines = 120;
			esp_err_t res = driver_ili9341_set_addr_window(0, y0, ILI9341_WIDTH, lines);
			if (res != ESP_OK) break;
			res = driver_ili9341_send(buffer+(y0*ILI9341_WIDTH)*2, ILI9341_WIDTH*lines*2, true);
			if (res != ESP_OK) break;
			y0 += lines;
			h -= lines;
		}
		return res;
	}
	
	//printf("Refreshing %ux%u area at (%u,%u) (%u,%u) using method 2\n", w, h, x0, y0, x1, y1);
	uint8_t *area = malloc(w*h*2);
	if (!area) return ESP_FAIL;
	for (uint16_t y = 0; y < h; y++) {
		uint32_t areaOffset = (y*w)*2;
		uint32_t fbOffset = (x0+(y0+y)*ILI9341_WIDTH)*2;
		memcpy(area+areaOffset, buffer+fbOffset, w*2);
	}
	res = driver_ili9341_set_addr_window(x0, y0, w, h);
	if (res == ESP_OK) {
		res = driver_ili9341_send(area, w*h*2, true);
	}
	free(area);
	return res;
}

#else
esp_err_t driver_ili9341_init(void) { return ESP_OK; } //Dummy function.
#endif // CONFIG_DRIVER_ILI9341_ENABLE
