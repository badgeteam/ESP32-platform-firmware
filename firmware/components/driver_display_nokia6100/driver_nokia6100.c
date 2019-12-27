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

#include "include/driver_nokia6100.h"

#ifdef CONFIG_DRIVER_NOKIA6100_ENABLE

#define NOKIA6100_MAX_LINES 8

static const char *TAG = "nokia6100";

uint8_t *internalBuffer; //Internal transfer buffer for doing partial updates

static spi_device_handle_t spi_bus = NULL;

#define CONFIG_DRIVER_NOKIA6100_PHILLIPS

const uint8_t nokia6100_init_data[] = {
#ifdef CONFIG_DRIVER_NOKIA6100_PHILLIPS
	SLEEPOUT, 0,
	BSTRON,   0,
	DISPON,   0,
	PCOLMOD,  1, 0x03,
	MADCTL,   1, 0xC0,
	SETCON,   1, 0x3f,
	NOPP,     0,
	0x00
#else
	DISCTL,   3, 0x0C, 0x20, 0x00,
	COMSCN,   1, 0x01,
	OSCON,    0,
	SLPOUT,   0,
	PWRCTR,   1, 0x0F,
	DISINV,   0,
	DATCTL,   3, 0x03, 0x00, 0x02,
	VOLCTR,   2, 32, 3,
	NOP,      0,
	0x00
#endif
};

inline uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

esp_err_t driver_nokia6100_send9(const uint8_t type, const uint8_t command)
{
	uint16_t data = (command>>1) | ((command&1)<<15) | (type<<7);
	spi_transaction_t t = {
		.length = 9,
		.tx_buffer = &data
	};
	return spi_device_transmit(spi_bus, &t);
}

esp_err_t driver_nokia6100_send_command(uint8_t cmd)
{
	//printf("NOKIA6100 send command 0x%02x\n", cmd);
	return driver_nokia6100_send9(0, cmd);
}

esp_err_t driver_nokia6100_send_data(uint8_t data)
{
	//printf("NOKIA6100 send data 0x%02x\n", data);
	return driver_nokia6100_send9(1, data);
}

esp_err_t driver_nokia6100_write_initData(const uint8_t * data)
{
	uint8_t cmd, len;
	while(true) {
		cmd = *data++;
		if(!cmd) return ESP_OK; //END
		len = *data++;
		driver_nokia6100_send_command(cmd);
		for (uint8_t i = 0; i < len; i++) driver_nokia6100_send_data(data[i]);
		data+=len;
	}
	return ESP_OK;
}

esp_err_t driver_nokia6100_reset(void) {
	#if CONFIG_PIN_NUM_NOKIA6100_RESET >= 0
	printf("Reset LOW\n");
	esp_err_t res = gpio_set_level(CONFIG_PIN_NUM_NOKIA6100_RESET, false);
	if (res != ESP_OK) return res;
	ets_delay_us(200000);
	printf("Reset HIGH\n");
	res = gpio_set_level(CONFIG_PIN_NUM_NOKIA6100_RESET, true);
	if (res != ESP_OK) return res;
	#endif
	ets_delay_us(200000);
	return ESP_OK;
}

esp_err_t driver_nokia6100_set_backlight(bool state)
{
	return gpio_set_level(CONFIG_PIN_NUM_NOKIA6100_BACKLIGHT, state);
}

esp_err_t driver_nokia6100_init(void)
{
	static bool driver_nokia6100_init_done = false;
	if (driver_nokia6100_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	esp_err_t res;
	
	//Initialize backlight GPIO pin
	res = gpio_set_direction(CONFIG_PIN_NUM_NOKIA6100_BACKLIGHT, GPIO_MODE_OUTPUT);
	if (res != ESP_OK) return res;

	//Turn off backlight
	res = driver_nokia6100_set_backlight(false);
	if (res != ESP_OK) return res;
	
	//Allocate partial update buffer
	internalBuffer = heap_caps_malloc(CONFIG_DRIVER_VSPI_MAX_TRANSFERSIZE, MALLOC_CAP_8BIT);
	if (!internalBuffer) return ESP_FAIL;
	
	//Initialize reset GPIO pin
	#if CONFIG_PIN_NUM_NOKIA6100_RESET >= 0
	res = gpio_set_direction(CONFIG_PIN_NUM_NOKIA6100_RESET, GPIO_MODE_OUTPUT);
	if (res != ESP_OK) return res;
	#endif
	
	static const spi_device_interface_config_t devcfg = {
		.clock_speed_hz = 8000 * 1000,
		.mode           = 3,  // SPI mode 3
		.cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
		.spics_io_num   = CONFIG_PIN_NUM_NOKIA6100_CS,
		.queue_size     = 50,
		.flags          = (SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE)
	};
	res = spi_bus_add_device(VSPI_HOST, &devcfg, &spi_bus);
	if (res != ESP_OK) return res;

	//Reset the LCD display
	res = driver_nokia6100_reset();
	if (res != ESP_OK) return res;

	//Send the initialization data to the LCD display
	res = driver_nokia6100_write_initData(nokia6100_init_data);
	if (res != ESP_OK) return res;

	//Turn on backlight (FIXME: remove this?)
	res = driver_nokia6100_set_backlight(true);
	if (res != ESP_OK) return res;
	
	driver_nokia6100_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

esp_err_t driver_nokia6100_write(const uint8_t *buffer)
{
	return driver_nokia6100_write_partial(buffer, 0, 0, NOKIA6100_WIDTH, NOKIA6100_HEIGHT);
}

esp_err_t driver_nokia6100_write_partial(const uint8_t *buffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	if (x0 > x1) return ESP_FAIL;
	if (y0 > y1) return ESP_FAIL;
	
	//HACK currently this sends one pixel per transaction, VERY slow!
	for (uint32_t x = x0; x < x1; x++) {
		for (uint32_t y = y0; y < y1; y++) {
			#ifdef CONFIG_DRIVER_NOKIA6100_PHILLIPS
				driver_nokia6100_send_command(PASETP);
			#else
				driver_nokia6100_send_command(PASET);
			#endif
			driver_nokia6100_send_data(y);
			driver_nokia6100_send_data(y);

			#ifdef CONFIG_DRIVER_NOKIA6100_PHILLIPS
				driver_nokia6100_send_command(CASETP);
			#else
				driver_nokia6100_send_command(CASET);
			#endif
			driver_nokia6100_send_data(x);
			driver_nokia6100_send_data(x);
			
			#ifdef CONFIG_DRIVER_NOKIA6100_PHILLIPS
				driver_nokia6100_send_command(RAMWRP);
			#else
				driver_nokia6100_send_command(RAMWR);
			#endif
			
			//HACK currently this expects a 16-bit framebuffer...
			uint16_t data = (buffer[(x+y*NOKIA6100_WIDTH)*2]<<8)+buffer[(x+y*NOKIA6100_WIDT
H)*2+1];
			//HACK ...of which we throw away the unused bits
			uint8_t b = data>>12;
			uint8_t g = (data>>7)&0xF;
			uint8_t r = (data>>1)&0xF;
			
			driver_nokia6100_send_data(((data>>4)&0xff));
			driver_nokia6100_send_data((data&0x0F)<<4 | (data>>8));
			driver_nokia6100_send_data(data&0xff);
			
			driver_nokia6100_send_data((r<<4)+g);
			driver_nokia6100_send_data((b<<4)+0);
			driver_nokia6100_send_command(NOPP);
		}
	}
	return ESP_OK;
}

#else
esp_err_t driver_nokia6100_init(void) { return ESP_OK; } //Dummy function.
#endif // CONFIG_DRIVER_NOKIA6100_ENABLE
