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

#error "Sorry, this driver is not finished *AT ALL*, can't continue."

#define NOKIA6100_MAX_LINES 8

static const char *TAG = "nokia6100";

uint8_t *internalBuffer; //Internal transfer buffer for doing partial updates

static spi_device_handle_t spi_bus = NULL;

//#define CONFIG_DRIVER_NOKIA6100_PHILLIPS

const uint8_t nokia6100_init_data[] = {
#ifdef CONFIG_DRIVER_NOKIA6100_PHILLIPS
	SLEEPOUT, 0,
	BSTRON,   0,
	DISPON,   0,
	PCOLMOD,  1, 0x03,
	MADCTL,   1, 0x00,
	SETCON,   1, 0x30,
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

esp_err_t driver_nokia6100_send9(const uint8_t type, const uint8_t command)
{
	uint16_t data = ((type<<8) | command);
	spi_transaction_t t = {
		.length = 9,
		.tx_buffer = &data
	};
	return spi_device_transmit(spi_bus, &t);
}

esp_err_t driver_nokia6100_send_command(uint8_t cmd)
{
	printf("NOKIA6100 send command 0x%02x\n", cmd);
	return driver_nokia6100_send9(0, cmd);
}

esp_err_t driver_nokia6100_send_data(uint8_t data)
{
	printf("NOKIA6100 send data 0x%02x\n", data);
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
		if (len > 0) driver_nokia6100_send_data(data[0]);
		data+=len;
	}
	return ESP_OK;
}

esp_err_t driver_nokia6100_reset(void) {
	#if CONFIG_PIN_NUM_NOKIA6100_RESET >= 0
	esp_err_t res = gpio_set_level(CONFIG_PIN_NUM_NOKIA6100_RESET, false);
	if (res != ESP_OK) return res;
	ets_delay_us(200000);
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

uint8_t *framespi = NULL;

void writeframe9(int offset, uint16_t value)
{
  int index = offset/8; // offset in framespi
  int shift = offset%8; // bitshift to apply
  uint8_t mask0 = 0xff<<(8-shift);
  uint8_t mask1 = 0xff>>(7-shift);

  framespi[index] = (framespi[index]&mask0)|(value>>1)>>shift;
  framespi[index+1] = (value&mask1) << (7-shift);
}

void writeframe(int nbits)
{
	printf("NOKIA6100 writeframe(%d)\n", nbits);
	spi_transaction_t t = {
		.length = (nbits/64)*64,
		.tx_buffer = &framespi
	};
	spi_device_transmit(spi_bus, &t);
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
		.clock_speed_hz = 1 * 1000 * 1000,
		.mode           = 0,  // SPI mode 0
		.spics_io_num   = CONFIG_PIN_NUM_NOKIA6100_CS,
		.queue_size     = 1,
		.flags          = (SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE),//SPI_DEVICE_HALFDUPLEX,
	};
	res = spi_bus_add_device(VSPI_HOST, &devcfg, &spi_bus);
	if (res != ESP_OK) return res;

	//Reset the LCD display
	res = driver_nokia6100_reset();
	if (res != ESP_OK) return res;

	//Send the initialization data to the LCD display
	res = driver_nokia6100_write_initData(nokia6100_init_data);
	if (res != ESP_OK) return res;

	//Turn on backlight
	res = driver_nokia6100_set_backlight(true);
	if (res != ESP_OK) return res;
	
	/* TEST */
	
	/*framespi = (uint8_t*) heap_caps_malloc(792*sizeof(uint8_t), MALLOC_CAP_DMA);
	
	driver_nokia6100_send_command(PASETP);
	driver_nokia6100_send_data(0);
	driver_nokia6100_send_data(131);
	
	driver_nokia6100_send_command(CASETP);
	driver_nokia6100_send_data(0);
	driver_nokia6100_send_data(131);
	
	driver_nokia6100_send_command(RAMWRP);
	
	uint16_t data = 0x00FF;
	
	int k = 0;
	for (uint32_t i = 0; i < NOKIA6100_WIDTH * NOKIA6100_HEIGHT; i++) {
		writeframe9(k, ((data>>4)&0xff) | (1 << 8));
		k+=9;
		writeframe9(k, (data&0x0F)<<4 | (data>>8) | (1 << 8));
		k+=9;
		writeframe9(k, (data&0xff) | (1 << 8));
		k+=9;
		
		i+=2;
		if (i%128==0) {
			writeframe(k);
			k = 0;
		}
	}*/
	
	/* TEST */
	
	uint16_t color = 0xF00;
	
	/*
	
	driver_nokia6100_send_command(0x00);//NOP for Phillips
	
	driver_nokia6100_send_command(PASETP);
	driver_nokia6100_send_data(20);
	driver_nokia6100_send_data(20);
	driver_nokia6100_send_command(CASETP);
	driver_nokia6100_send_data(20);
	driver_nokia6100_send_data(20);
	driver_nokia6100_send_command(RAMWRP);
	driver_nokia6100_send_data((color>>4)&0x00FF);
	driver_nokia6100_send_data(((color&0x0F)<<4));*/
	
	driver_nokia6100_send_command(PASET);
	driver_nokia6100_send_data(0);
	driver_nokia6100_send_data(131);
	driver_nokia6100_send_command(CASET);
	driver_nokia6100_send_data(0);
	driver_nokia6100_send_data(131);
	driver_nokia6100_send_command(RAMWR);
	

	for(unsigned int i=0; i < (131*131)/2; i++) {
		driver_nokia6100_send_data((color>>4)&0x00FF);
		driver_nokia6100_send_data(((color&0x0F)<<4)|(color>>8));
		driver_nokia6100_send_data(color&0x0FF);
	}

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
	uint16_t w = x1-x0;
	uint16_t h = y1-y0;
	return ESP_OK;
}

#else
esp_err_t driver_nokia6100_init(void) { return ESP_OK; } //Dummy function.
#endif // CONFIG_DRIVER_NOKIA6100_ENABLE
