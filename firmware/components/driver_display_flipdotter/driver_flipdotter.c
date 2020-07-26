#include <sdkconfig.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <esp_err.h>

#include <driver/spi_master.h>
#include <freertos/task.h>

#include "driver/ledc.h"

#include "include/driver_flipdotter.h"

#ifdef CONFIG_DRIVER_FLIPDOTTER_ENABLE

#define TAG "flipdotter"

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
bool __state_unknown  = false;

uint8_t* stateCurrent = NULL;
const uint8_t* stateTarget  = NULL;

xSemaphoreHandle driver_flipdotter_refresh_trigger = NULL;

inline bool _get_pixel(uint8_t col, uint8_t row, uint8_t mod, const uint8_t* buffer)
{
	//1BPP horizontal
	//uint32_t position = (row * (FLIPDOTTER_WIDTH / 8)) + ((col + (mod * CONFIG_FLIPDOTTER_COLS)) / 8);
	//uint8_t  bit      = ((col + (mod * CONFIG_FLIPDOTTER_COLS))) % 8;

	uint16_t x = col;
	uint16_t y = row;
	
	uint32_t position = ( (y / 8) * FLIPDOTTER_WIDTH) + x + (mod * CONFIG_FLIPDOTTER_COLS);
	uint8_t  bit      = y % 8;
	return !((buffer[position] >> bit)&0x01);
}

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

void driver_flipdotter_refresh_task(void *arg)
{
	while (1) { //This function runs as a FreeRTOS task, that is why it has it's own "main" loop.
		if (xSemaphoreTake(driver_flipdotter_refresh_trigger, portMAX_DELAY)) { //This check lets FreeRTOS pause this task until we get permission to access the "shared resource", which in this case is the refresh trigger
			if (__current_col >= CONFIG_FLIPDOTTER_COLS) {
				__current_col = 0;
				__current_row++;
			}
			if (__current_row >= CONFIG_FLIPDOTTER_ROWS) {
				__current_row = 0;
				__current_mod++;
			}
			if (__current_mod >= CONFIG_FLIPDOTTER_MODULES) {
				//Done
				memcpy(stateCurrent, stateTarget, FLIPDOTTER_BUFFER_SIZE);
				__state_unknown = false;
				if (__queue) {
					// Start over
					__queue = false;
					__current_col = 0;
					__current_row = 0;
					__current_mod = 0;
					xSemaphoreGive(driver_flipdotter_refresh_trigger); //Continue
				} else {
					// Stop
					//printf("refresh: done\n");
					__busy = false;
				}
			} else {
				bool currentValue = _get_pixel( __current_col, __current_row, __current_mod, stateCurrent);
				bool targetValue  = _get_pixel( __current_col, __current_row, __current_mod, stateTarget);
				
				if ((currentValue == targetValue) && (!__state_unknown)) {
					//printf("%d, %d, %d: -\n", __current_col, __current_row, __current_mod);
					__current_col++; //Go to the next pixel
					xSemaphoreGive(driver_flipdotter_refresh_trigger); //Continue, give ourselves the refresh trigger
				} else {
					//printf("%d, %d, %d: %d\n", __current_col, __current_row, __current_mod, targetValue);
					driver_flipdotter_set_pixel(__current_col, __current_row, __current_mod, targetValue);
					__current_col++; //Go to the next pixel
					//Note: once the SPI transaction is completed the post transfer call back will give us back the refresh trigger semaphore, this task will thus wait for the end of the transfer.
				}
			}
		}
	}
}

esp_err_t driver_flipdotter_write(const uint8_t *buffer)
{
	stateTarget = buffer;
	if (__busy) {
		//If the display is already being refreshed then we tell the driver to start over once done
		__queue = true;
		printf("Flipdot is busy, queued refresh cycle\n");
	} else {
		//If the display is idle then we start the refresh cycle
		__busy = true;
		__queue = false;
		__current_col = 0;
		__current_row = 0;
		__current_mod = 0;
		xSemaphoreGive(driver_flipdotter_refresh_trigger);
	}
	return ESP_OK;
}

static void driver_flipdotter_post_transfer_callback(spi_transaction_t *t)
{
	gpio_set_level(CONFIG_PIN_NUM_FLIPDOTTER_FIRE, true);
	ets_delay_us(250);
	gpio_set_level(CONFIG_PIN_NUM_FLIPDOTTER_FIRE, false);
	xSemaphoreGiveFromISR(driver_flipdotter_refresh_trigger, NULL);
}

esp_err_t driver_flipdotter_set_backlight(uint8_t brightness)
{
	#if CONFIG_PIN_NUM_FLIPDOTTER_BACKLIGHT >= 0
		//return gpio_set_level(CONFIG_PIN_NUM_FLIPDOTTER_BACKLIGHT, brightness > 127);
		ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, brightness);
		ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
		return ESP_OK;
	#else
		return ESP_FAIL;
	#endif
}

esp_err_t driver_flipdotter_init(void)
{
	static bool driver_flipdotter_init_done = false;
	if (driver_flipdotter_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	esp_err_t res;
	
	// Initialize backlight GPIO pin
	#if CONFIG_PIN_NUM_FLIPDOTTER_BACKLIGHT >= 0
		res = gpio_set_direction(CONFIG_PIN_NUM_FLIPDOTTER_BACKLIGHT, GPIO_MODE_OUTPUT);
		if (res != ESP_OK) return res;
		
		ledc_timer_config_t ledc_timer = {
			.duty_resolution = LEDC_TIMER_8_BIT, // resolution of PWM duty
			.freq_hz = 5000,                     // frequency of PWM signal
			.speed_mode = LEDC_HIGH_SPEED_MODE,  // timer mode
			.timer_num = LEDC_TIMER_0,           // timer index
		};
		ledc_timer_config(&ledc_timer);
		
		ledc_channel_config_t ledc_channel = {
			.channel    = LEDC_CHANNEL_0,
			.duty       = 0,
			.gpio_num   = CONFIG_PIN_NUM_FLIPDOTTER_BACKLIGHT,
			.speed_mode = LEDC_HIGH_SPEED_MODE,
			.hpoint     = 0,
			.timer_sel  = LEDC_TIMER_0
		};
		ledc_channel_config(&ledc_channel);
	#endif
	
	//Initialize fire GPIO pin
	res = gpio_set_direction(CONFIG_PIN_NUM_FLIPDOTTER_FIRE, GPIO_MODE_OUTPUT);
	if (res != ESP_OK) return res;
	res = gpio_set_level(CONFIG_PIN_NUM_FLIPDOTTER_FIRE, false);
	if (res != ESP_OK) return res;

	//Initialize SPI device
	static const spi_device_interface_config_t devcfg = {
		.clock_speed_hz = 32000000,
		.mode           = 0,
		.spics_io_num   = CONFIG_PIN_NUM_FLIPDOTTER_LATCH,
		.queue_size     = 1,
		.flags          = 0,
		.post_cb        = driver_flipdotter_post_transfer_callback
	};
	res = spi_bus_add_device(VSPI_HOST, &devcfg, &spi_device);
	if (res != ESP_OK) return res;
	
	//Allocate buffer for current state
	stateCurrent = malloc(FLIPDOTTER_BUFFER_SIZE);
	if (!stateCurrent) return ESP_FAIL;
	memset(stateCurrent, 0x00, FLIPDOTTER_BUFFER_SIZE);
	__state_unknown = true;
	
	//Initialize semaphore for refresh task
	driver_flipdotter_refresh_trigger = xSemaphoreCreateBinary();
	if (driver_flipdotter_refresh_trigger == NULL) return ESP_ERR_NO_MEM;
	
	//Create the refresh task
	xTaskCreate(&driver_flipdotter_refresh_task, "Flipdot display refresh task", 4096, NULL, 10, NULL);
	
	driver_flipdotter_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

#else // CONFIG_DRIVER_VSPI_ENABLE
esp_err_t driver_flipdotter_init(void) { return ESP_OK; } // Dummy function, leave empty!
#endif
