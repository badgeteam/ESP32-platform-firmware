#include <sdkconfig.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_sleep.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "include/driver_ledmatrix.h"

#ifdef CONFIG_DRIVER_LEDMATRIX_ENABLE
#define TAG "ledmatrix"

//uint8_t rate = 60;

const uint8_t rows[] = {DRIVER_LEDMATRIX_ROWS};
const uint8_t columns[] = {DRIVER_LEDMATRIX_COLUMNS};

const uint8_t* buffer;

void displayTask(void* arg)
{
	while (1) {
		if (!buffer) {
			vTaskDelay(1 / portTICK_PERIOD_MS);
			continue;
		}
		for (uint8_t s = 0; s < 0x3F; s+=4) {
			for (uint8_t c = 0; c < sizeof(columns); c++) {
				//TickType_t xLastWakeTime = xTaskGetTickCount();
				int any = 0;
				for (uint8_t r = 0; r < sizeof(rows); r++) {
					bool state = buffer[r*sizeof(columns) + c] > s;
					if (state) any++;
					gpio_set_level(rows[r], state);
				}
				if (any) gpio_set_level(columns[c], true);
				//vTaskDelay(1 / portTICK_PERIOD_MS);
				ets_delay_us(1);
				gpio_set_level(columns[c], false);
			}
			ets_delay_us(5);
		}
	}
}

esp_err_t driver_ledmatrix_init(void)
{
	// Use a static boolean to make sure we only initialise the driver once
	static bool driver_ledmatrix_init_done = false;
	if (driver_ledmatrix_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	esp_err_t res;
	
	// Initialise the GPIO pins used as row outputs
	for (uint8_t i = 0; i < sizeof(rows); i++) {
		uint8_t pin = rows[i];
		printf("[LM] Using pin %u as row output #%u.\n", pin, i);
		gpio_pad_select_gpio(pin);
		res = gpio_set_direction(pin, GPIO_MODE_OUTPUT);
		if (res != ESP_OK) return res;
		gpio_set_level(pin, false);
	}
	
	// Initialise the GPIO pins used as column outputs
	for (uint8_t i = 0; i < sizeof(columns); i++) {
		uint8_t pin = columns[i];
		printf("[LM] Using pin %u as column output #%u.\n", pin, i);
		gpio_pad_select_gpio(pin);
		res = gpio_set_direction(pin, GPIO_MODE_OUTPUT);
		if (res != ESP_OK) return res;
		gpio_set_level(pin, false);
	}
	
	// Create a task pinned to core 1 which will handle writing to the display
	xTaskCreatePinnedToCore(
		&displayTask, /* Task function. */
		"ledmatrix",  /* String with name of task. */
		1024,         /* Stack size in words. */
		NULL,         /* Parameter passed as input of the task */
		1,            /* Priority of the task. (Lower = more important) */
		NULL,         /* Task handle. */
		1             /* Core ID */
	);
	
	// Set the static boolean indicating that we completed initialisation of the driver
	driver_ledmatrix_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

esp_err_t driver_ledmatrix_set_buffer(const uint8_t *newBuffer)
{
	buffer = newBuffer;
	return ESP_OK;
}

#else
esp_err_t driver_ledmatrix_init(void) { return ESP_OK; } // Dummy function, leave empty!
#endif
