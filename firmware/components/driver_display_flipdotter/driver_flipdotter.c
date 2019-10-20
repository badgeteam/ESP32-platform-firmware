#include <sdkconfig.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <esp_err.h>

#include <driver/spi_master.h>
#include <freertos/task.h>

#include "include/driver_flipdotter.h"

#ifdef CONFIG_DRIVER_FLIPDOTTER_ENABLE

#define TAG "flipdotter"

#define BUFFER_SIZE (CONFIG_FLIPDOTTER_COLS*CONFIG_FLIPDOTTER_ROWS*CONFIG_FLIPDOTTER_MODULES) / 8
#define WIDTH CONFIG_FLIPDOTTER_COLS*CONFIG_FLIPDOTTER_MODULES
#define HEIGHT CONFIG_FLIPDOTTER_ROWS

const unsigned char index_to_bitpattern_map[28] = {
	1,   2,  3,  4,  5,  6,  7,
	9,  10, 11, 12, 13, 14, 15,
	17, 18, 19, 20, 21, 22, 23,
	25, 26, 27, 28, 29, 30, 31
};

static spi_device_handle_t spi_device = NULL;
uint8_t __current_col = 0;
uint8_t __current_row = 0;
uint8_t __current_mod = 0;
bool __busy           = false;
bool __queue          = false;

uint8_t* stateCurrent = NULL;
uint8_t* stateTarget  = NULL;
uint8_t* stateQueue   = NULL;

inline bool _get_pixel(uint8_t col, uint8_t row, uint8_t mod, uint8_t* buffer)
{
	//1BPP horizontal
	uint32_t position = (row * (WIDTH/8)) + (col / 8);
	uint8_t  bit      = col % 8;
	return buffer[position] >> bit;
}

/*esp_err_t driver_flipdotter_refresh()
{
	if (__busy) { //A refresh is already in progress
		__queue = true;
		return ESP_OK;
	}
	
	memcpy(stateTarget, stateQueue, BUFFER_SIZE);
	
	__busy = true;
	__queue = false;
	__current_col = 0;
	__current_row = 0;
	__current_mod = 0;
	_next_pixel();
}

void _next_pixel()
{
	if (!__busy) return;
	
	if ((__current_mod >= CONFIG_FLIPDOTTER_MODULES) {
		//Done
		memcpy(stateCurrent, stateTarget, BUFFER_SIZE);
		__busy = false;
		if (__queue) driver_flipdotter_refresh();
		return;
	}
	
	if (
}*/

esp_err_t driver_flipdotter_set_pixel(uint8_t col, uint8_t row, uint8_t mod, bool color)
{
	uint8_t out[2] = {0,0}; //(col&0x1F) | (color<<7), (row&0x1F) | (mod<<5) };
	
	for (uint8_t i = 0; i < 5; i++) {
		out[0] |= index_to_bitpattern_map[col]&(1<<i);
		out[1] |= index_to_bitpattern_map[row]&(1<<i);
	}
	
	out[0] |= color << 7;
	out[1] |= mod   << 5;
	
	uint8_t in[2]; //Dummy.

	spi_transaction_t t = {
		.flags = 0,
		.length = 8 * 2,
		.tx_buffer = out,
		.rx_buffer = in  
	};

	return spi_device_transmit(spi_device, &t);
}

static void driver_flipdotter_post_transfer_callback(spi_transaction_t *t)
{
	gpio_set_level(CONFIG_PIN_NUM_FLIPDOTTER_FIRE, true);
	ets_delay_us(200);
	gpio_set_level(CONFIG_PIN_NUM_FLIPDOTTER_FIRE, false);
}

esp_err_t driver_flipdotter_init(void)
{
	static bool driver_flipdotter_init_done = false;
	if (driver_flipdotter_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	esp_err_t res;
	
	//Initialize fire GPIO pin
	res = gpio_set_direction(CONFIG_PIN_NUM_FLIPDOTTER_FIRE, GPIO_MODE_OUTPUT);
	if (res != ESP_OK) return res;
	res = gpio_set_level(CONFIG_PIN_NUM_FLIPDOTTER_FIRE, false);
	if (res != ESP_OK) return res;

	//Initialize SPI device
	static const spi_device_interface_config_t devcfg = {
		.clock_speed_hz = 16000000,
		.mode           = 0,
		.spics_io_num   = CONFIG_PIN_NUM_FLIPDOTTER_LATCH,
		.queue_size     = 1,
		.flags          = 0,
		.post_cb        = driver_flipdotter_post_transfer_callback
	};
	res = spi_bus_add_device(VSPI_HOST, &devcfg, &spi_device);
	if (res != ESP_OK) return res;
	
	/*stateCurrent = malloc(BUFFER_SIZE);
	if (!stateCurrent) return ESP_FAIL;
	stateTarget  = malloc(BUFFER_SIZE);
	if (!stateTarget) return ESP_FAIL;
	
	memset(stateCurrent, 0x00, BUFFER_SIZE);
	memset(stateTarget,  0xFF, BUFFER_SIZE);*/
	
	//driver_flipdotter_refresh();
	
	/* TEST */
	while (1) {
		uint8_t mod = 0;
		//for (uint8_t mod = 0; mod < CONFIG_FLIPDOTTER_MODULES; mod++) {
			for (uint8_t row = 0; row < CONFIG_FLIPDOTTER_ROWS; row++) {
				for (uint8_t col = 0; col < CONFIG_FLIPDOTTER_COLS; col++) {
					//printf("%d, %d, %d\n", col, row, mod);
					driver_flipdotter_set_pixel(col,row,mod,false);
					vTaskDelay(5 / portTICK_PERIOD_MS);
					driver_flipdotter_set_pixel(col,row,mod,true);
					vTaskDelay(5 / portTICK_PERIOD_MS);
				}
			}
		//}
	}
	
	
	driver_flipdotter_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

#else // CONFIG_DRIVER_VSPI_ENABLE
esp_err_t driver_flipdotter_init(void) { return ESP_OK; } // Dummy function, leave empty!
#endif
