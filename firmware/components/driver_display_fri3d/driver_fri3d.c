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
#include "include/driver_fri3d.h"

#define BRIGHTNESS_STEPS 32

#ifdef CONFIG_DRIVER_FRI3D_ENABLE

const uint8_t* internalBuffer = NULL;

static const char *TAG = "fri3d";

static spi_device_handle_t spi_bus = NULL;

uint8_t framerate = 90;

esp_err_t driver_fri3d_send_line(uint8_t x, uint8_t brightnessPos)
{
	uint8_t a = 0;
	uint8_t b = 0;
	uint8_t c = 0x7F;
	if (x < 7) {
		if (internalBuffer) {
			for (uint8_t i = 0; i < 7; i++) {
				if (internalBuffer[(FRI3D_WIDTH*(4-i)) + x + 0] > (BRIGHTNESS_STEPS - brightnessPos)*(256/BRIGHTNESS_STEPS)) a |= (1<<i);
				if (internalBuffer[(FRI3D_WIDTH*(4-i)) + x + 7] > (BRIGHTNESS_STEPS - brightnessPos)*(256/BRIGHTNESS_STEPS)) b |= (1<<i);
			}
		}
		c = (1<<x) ^ 0x7F;
	}
	
	a = a & 0x1F;
	b = b & 0x1F;	
	
	uint32_t data = a | (c << 5) | (c << 17) | (b << 12);
	
	uint8_t buffer[3];
	buffer[0] = (data>>16)&0xFF;
	buffer[1] = (data>> 8)&0xFF;
	buffer[2] = data      &0xFF;
	
	spi_transaction_t t = {
		.length = 24,
		.tx_buffer = buffer
	};
	return spi_device_transmit(spi_bus, &t);
}

static void driver_fri3d_spi_pre_transfer_callback(spi_transaction_t *t)
{
	gpio_set_level(CONFIG_PIN_NUM_FRI3D_LAT, false);
}

static void driver_fri3d_spi_post_transfer_callback(spi_transaction_t *t)
{
	gpio_set_level(CONFIG_PIN_NUM_FRI3D_LAT, true);
}

void displayTask(void *pvParameter)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(1) {
		vTaskDelayUntil( &xLastWakeTime, 1.0 / framerate * 1000 / portTICK_PERIOD_MS );
		for (uint8_t j = 0; j < BRIGHTNESS_STEPS; j++) {
			for (uint8_t i = 0; i < 8; i++) driver_fri3d_send_line(i, j);
		}
	}
}

esp_err_t driver_fri3d_init(void)
{
	static bool driver_fri3d_init_done = false;
	if (driver_fri3d_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	esp_err_t res;
	
	//Initialize OE GPIO pin
	gpio_pad_select_gpio(CONFIG_PIN_NUM_FRI3D_OE);
	res = gpio_set_direction(CONFIG_PIN_NUM_FRI3D_OE, GPIO_MODE_OUTPUT);
	if (res != ESP_OK) return res;
	gpio_set_level(CONFIG_PIN_NUM_FRI3D_OE, false);
	
	//Initialize LAT GPIO pin
	gpio_pad_select_gpio(CONFIG_PIN_NUM_FRI3D_LAT);
	res = gpio_set_direction(CONFIG_PIN_NUM_FRI3D_LAT, GPIO_MODE_OUTPUT);
	if (res != ESP_OK) return res;
	gpio_set_level(CONFIG_PIN_NUM_FRI3D_LAT, true);
	
	static const spi_bus_config_t buscfg = {
		.mosi_io_num     = CONFIG_PIN_NUM_FRI3D_DAT,
		.sclk_io_num     = CONFIG_PIN_NUM_FRI3D_CLK,
		.miso_io_num     = -1,
		.quadwp_io_num   = -1,
		.quadhd_io_num   = -1,
		.max_transfer_sz = 24,
	};
	
	//Initialize the SPI bus
	res = spi_bus_initialize(VSPI_HOST, &buscfg, 2);
	if (res != ESP_OK) return res;
	
	static const spi_device_interface_config_t devcfg = {
		.clock_speed_hz = 10 * 1000 * 1000,
		.mode           = 0,  // SPI mode 0
		.spics_io_num   = -1,
		.queue_size     = 1,
		.flags          = (SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE),
		.pre_cb         = driver_fri3d_spi_pre_transfer_callback,
		.post_cb        = driver_fri3d_spi_post_transfer_callback
	};
	
	//Add the device to the SPI bus
	res = spi_bus_add_device(VSPI_HOST, &devcfg, &spi_bus);
	
	printf("Starting fri3d task...\n");
	
	xTaskCreatePinnedToCore(
		&displayTask, /* Task function. */
		"display",    /* String with name of task. */
		1024,         /* Stack size in words. */
		NULL,         /* Parameter passed as input of the task */
		1,            /* Priority of the task. (Lower = more important) */
		NULL,         /* Task handle. */
		1             /* Core ID */
	);
	
	driver_fri3d_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

esp_err_t driver_fri3d_write(const uint8_t *buffer)
{
	internalBuffer = buffer;
	return ESP_OK;
}

#else
esp_err_t driver_fri3d_init(void) { return ESP_OK; } //Dummy function.
#endif // CONFIG_DRIVER_FRI3D_ENABLE
