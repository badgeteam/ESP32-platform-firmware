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

#include "include/driver_gxgde0213b1.h"

#ifdef CONFIG_DRIVER_GXGDE0213B1_ENABLE

#define GXGDE0213B1_MAX_TRANSFERSIZE GXGDE0213B1_BUFFER_SIZE / 8

static const char *TAG = "GXGDE0213B1";

uint8_t *internalBuffer; //Internal transfer buffer for doing partial updates

static spi_device_handle_t spi_bus = NULL;

static void driver_gxgde0213b1_spi_pre_transfer_callback(spi_transaction_t *t)
{
	uint8_t dc_level = *((uint8_t *) t->user);
	gpio_set_level(CONFIG_PIN_NUM_GXGDE0213B1_DCX, (int) dc_level);
}

const uint8_t gxgde0213b1_init_data[] = {
	0x01, 3, 249, 0x00, 0x00, //GDOControl
	0x0c, 3, 0xd7, 0xd6, 0x9d, //softstart
	0x2c, 1, 0xa8,
	0x3a, 1, 0x1a,
	0x3b, 1, 0x08,
	0x11, 1, 0x01,
	0x44, 2, 0x00, 0x0f,
	0x45, 4, 249, 0,0,0,
	0x4e, 1, 0,
	0x4f, 2, 249, 0,
	0x32, 29, 0x22, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x01, 0x00, 0x00, 0x00, 0x00,
	0x22, 1, 0xc0,
	0x20, 0,
	0x00
};

esp_err_t driver_gxgde0213b1_send(const uint8_t *data, int len, const uint8_t dc_level)
{
	if (len == 0) return ESP_OK;
	spi_transaction_t t = {
		.length = len * 8,  // transaction length is in bits
		.tx_buffer = data,
		.user = (void *) &dc_level,
	};
	return spi_device_transmit(spi_bus, &t);
}

esp_err_t driver_gxgde0213b1_receive(uint8_t *data, int len, const uint8_t dc_level)
{
	if (len == 0) return ESP_OK;
	spi_transaction_t t = {
		.length = len * 8,  // transaction length is in bits
		.rxlength = len * 8,
		.rx_buffer = data,
		.user = (void *) &dc_level,
	};
	return spi_device_transmit(spi_bus, &t);
}

esp_err_t driver_gxgde0213b1_write_initData(const uint8_t * data)
{
	uint8_t cmd, len;
	while(true) {
		cmd = *data++;
		if(!cmd) return ESP_OK; //END
		len = *data++;
		driver_gxgde0213b1_send(&cmd, 1, false);
		driver_gxgde0213b1_send(data, len, true);
		data+=len;
	}
	return ESP_OK;
}

esp_err_t driver_gxgde0213b1_send_command(uint8_t cmd)
{
	return driver_gxgde0213b1_send(&cmd, 1, false);
}

esp_err_t driver_gxgde0213b1_send_u32(uint32_t data)
{
	uint8_t buffer[4];
	buffer[0] = (data>>24)&0xFF;
	buffer[1] = (data>>16)&0xFF;
	buffer[2] = (data>> 8)&0xFF;
	buffer[3] = data      &0xFF;
	return driver_gxgde0213b1_send(buffer, 4, true);
}

esp_err_t driver_gxgde0213b1_reset(void) {
	#if CONFIG_PIN_NUM_GXGDE0213B1_RESET >= 0
	esp_err_t res = gpio_set_level(CONFIG_PIN_NUM_GXGDE0213B1_RESET, false);
	if (res != ESP_OK) return res;
	ets_delay_us(200000); //200ms
	res = gpio_set_level(CONFIG_PIN_NUM_GXGDE0213B1_RESET, true);
	if (res != ESP_OK) return res;
	#endif
	ets_delay_us(200000); //200ms
	return ESP_OK;
}

bool driver_gxgde0213b1_is_busy(void)
{
	return gpio_get_level(CONFIG_PIN_NUM_GXGDE0213B1_BUSY);
}

bool driver_gxgde0213b1_busy_wait(void)
{
	printf("Waiting for E-ink...\n");
	while (driver_gxgde0213b1_is_busy()) {
		ets_delay_us(10000); //10ms
	}
	printf("E-ink is done.\n");
	return true; //To-do: add timeout
}

esp_err_t driver_gxgde0213b1_init(void)
{
	static bool driver_gxgde0213b1_init_done = false;
	if (driver_gxgde0213b1_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	esp_err_t res;
	
	//Initialize busy GPIO pin
	res = gpio_set_direction(CONFIG_PIN_NUM_GXGDE0213B1_BUSY, GPIO_MODE_INPUT);
	if (res != ESP_OK) return res;
	
	//Allocate partial update buffer
	internalBuffer = heap_caps_malloc(GXGDE0213B1_MAX_TRANSFERSIZE, MALLOC_CAP_8BIT);
	if (!internalBuffer) return ESP_FAIL;
	
	//Initialize reset GPIO pin
	#if CONFIG_PIN_NUM_GXGDE0213B1_RESET >= 0
	res = gpio_set_direction(CONFIG_PIN_NUM_GXGDE0213B1_RESET, GPIO_MODE_OUTPUT);
	if (res != ESP_OK) return res;
	#endif
	
	//Initialize data/clock select GPIO pin
	res = gpio_set_direction(CONFIG_PIN_NUM_GXGDE0213B1_DCX, GPIO_MODE_OUTPUT);
	if (res != ESP_OK) return res;
	
	//Add the SPI device to the bus
	static const spi_device_interface_config_t devcfg = {
		.clock_speed_hz = 10 * 1000 * 1000,
		.mode           = 0,  // SPI mode 0
		.spics_io_num   = CONFIG_PIN_NUM_GXGDE0213B1_CS,
		.queue_size     = 1,
		.flags          = (SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE),
		.pre_cb         = driver_gxgde0213b1_spi_pre_transfer_callback, // Specify pre-transfer callback to handle D/C line
	};
	res = spi_bus_add_device(VSPI_HOST, &devcfg, &spi_bus);
	if (res != ESP_OK) return res;
	
	//Reset the LCD display
	res = driver_gxgde0213b1_reset();
	if (res != ESP_OK) return res;
	
	//Send the initialization data to the LCD display
	res = driver_gxgde0213b1_write_initData(gxgde0213b1_init_data);
	if (res != ESP_OK) return res;
	
	//Wait for idle
	driver_gxgde0213b1_busy_wait();
	
	driver_gxgde0213b1_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

esp_err_t driver_gxgde0213b1_write(const uint8_t *buffer)
{
	esp_err_t res;
	res = driver_gxgde0213b1_send_command(0x24);
	if (res != ESP_OK) return res;
	for (uint8_t i = 0; i < 8; i++) {
		res = driver_gxgde0213b1_send(buffer+(GXGDE0213B1_MAX_TRANSFERSIZE*i), GXGDE0213B1_MAX_TRANSFERSIZE, true);
	}
	if (res != ESP_OK) return res;
	res = driver_gxgde0213b1_send_command(0x22);
	if (res != ESP_OK) return res;
	uint8_t data[] = {0xc4};
	res = driver_gxgde0213b1_send(data, 1, true);
	if (res != ESP_OK) return res;
	res = driver_gxgde0213b1_send_command(0x20);
	if (res != ESP_OK) return res;
	driver_gxgde0213b1_busy_wait();
	res = driver_gxgde0213b1_send_command(0xFF);
	if (res != ESP_OK) return res;
	return ESP_OK;
}

#else
esp_err_t driver_gxgde0213b1_init(void) { return ESP_OK; } //Dummy function.
#endif
