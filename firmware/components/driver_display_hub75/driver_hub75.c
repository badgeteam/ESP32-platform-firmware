#include <sdkconfig.h>
#include <esp_err.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_heap_caps.h"
#include "include/val2pwm.h"
#include "driver/adc.h"

#include "include/i2s_parallel.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "freertos/queue.h"
#include "include/driver_hub75.h"
#include "include/driver_hub75_bits.h"

#include "include/compositor.h"
#include "esp_log.h"

#ifdef CONFIG_DRIVER_HUB75_ENABLE

#define TAG "hub75"

int brightness=CONFIG_HUB75_DEFAULT_BRIGHTNESS;
int framerate=20;
Color *hub75_framebuffer = NULL;

bool driver_hub75_active; // Stops all compositing + DMA buffer updating
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

void displayTask(void *pvParameter)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();

        // Turns on LED power
        gpio_reset_pin(GPIO_NUM_12);
        gpio_set_direction(GPIO_NUM_12, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_NUM_12, 1);

	while(driver_hub75_active) {
            if(compositor_status()) composite();
            uint32_t total_intensity = driver_hub75_render(brightness, hub75_framebuffer);
            vTaskDelayUntil( &xLastWakeTime, 1.0 / framerate * 1000 / portTICK_PERIOD_MS );
	}
	vTaskDelete( NULL );
}

Color* getFrameBuffer()
{
        return hub75_framebuffer;
}

esp_err_t driver_hub75_init(void)
{
	static bool driver_hub75_init_done = false;
	if (driver_hub75_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");

	//Change to set the global brightness of the display, range 1-63
	//Warning when set too high: Do not look into LEDs with remaining eye.
	#ifndef CONFIG_DRIVER_FRAMEBUFFER_ENABLE
	hub75_framebuffer = calloc(HUB75_BUFFER_SIZE, sizeof(uint8_t));
	#endif

	driver_hub75_init_bits();

	compositor_init();
	#ifndef CONFIG_DRIVER_FRAMEBUFFER_ENABLE
	compositor_setBuffer(getFrameBuffer());
	#endif

	driver_hub75_active = true;

	xTaskCreatePinnedToCore(
		&displayTask, /* Task function. */
		"display",    /* String with name of task. */
		4096,         /* Stack size in words. */
		NULL,         /* Parameter passed as input of the task */
		1,            /* Priority of the task. (Lower = more important) */
		NULL,         /* Task handle. */
		1             /* Core ID */
	);

	driver_hub75_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

void driver_hub75_set_brightness(int brightness_val)
{
	brightness = min(max(255, brightness_val), 65535);
}

void driver_hub75_set_framerate(int framerate_val)
{
	framerate = min(max(1, framerate_val), 30);
}

void driver_hub75_switch_buffer(uint8_t* buffer)
{
	hub75_framebuffer = (Color*) buffer;
	compositor_setBuffer(hub75_framebuffer);
}

#else // DRIVER_HUB75_ENABLE
esp_err_t driver_hub75_init(void) { return ESP_OK; } // Dummy function, leave empty!
#endif
