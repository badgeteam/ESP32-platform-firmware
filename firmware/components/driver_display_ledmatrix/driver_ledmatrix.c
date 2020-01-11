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

#define LEDMATRIX_TASK_STACK_LEN 1024
#define LEDMATRIX_TASK_PRIORITY  5
#define LEDMATRIX_TASK_CPU       1

#ifdef CONFIG_DRIVER_LEDMATRIX_ENABLE
#define TAG "ledmatrix"

void ledmatrix_display_task (void* arg)
{	
    while (1) {
	    //to-do: gpio flipping here :P
		vTaskDelay(1 / portTICK_PERIOD_MS); //Wait 1ms
	}

}

esp_err_t driver_ledmatrix_init(void)
{
	static bool driver_ledmatrix_init_done = false;
	if (driver_ledmatrix_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	//to-do: gpio init
	
#ifdef LEDMATRIX_TASK_CPU
	xTaskCreatePinnedToCore(ledmatrix_display_task, "ledmatrix", LEDMATRIX_TASK_STACK_LEN, NULL, LEDMATRIX_TASK_PRIORITY, NULL, LEDMATRIX_TASK_CPU);
#else
	xTaskCreate(ledmatrix_display_task, "ledmatrix", LEDMATRIX_TASK_STACK_LEN, NULL, LEDMATRIX_TASK_PRIORITY, NULL);
#endif
	
	driver_ledmatrix_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

#else
esp_err_t driver_ledmatrix_init(void) { return ESP_OK; } // Dummy function, leave empty!
#endif
