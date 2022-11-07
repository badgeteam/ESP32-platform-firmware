#include <sdkconfig.h>
#include <esp_err.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_heap_caps.h"
#include "include/val2pwm.h"
#include "driver/ledc.h"
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

// Configuration for LED power rail regulation
#define UPPER   600
#define MID     350
#define LOWER   40
#define PCT_MID 200  // 78% of 255
#define PCT_MIN 25  // 10% of 255

int brightness=CONFIG_HUB75_DEFAULT_BRIGHTNESS;
int framerate=20;
Color *hub75_framebuffer = NULL;

bool driver_hub75_active; // Stops all compositing + DMA buffer updating
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

void displayTask(void *pvParameter)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();

        ledc_timer_config_t ledc_timer = {
            .duty_resolution = LEDC_TIMER_8_BIT, // resolution of PWM duty
            .freq_hz = 20000,                      // frequency of PWM signal
            .speed_mode = LEDC_HIGH_SPEED_MODE,           // timer mode
            .timer_num = LEDC_TIMER_0,            // timer index
        };
        ledc_timer_config(&ledc_timer);
        ledc_channel_config_t led_power_dimmer = {
            .channel    = 0,
            .duty       = 0,
            .gpio_num   = 12,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_TIMER_0
        };
        ledc_channel_config(&led_power_dimmer);

        bool usb_connected = true;
        TickType_t lastUsbCheck = 0;
        uint8_t duty = 255;
        uint32_t deduced_current = 0;

	while(driver_hub75_active) {
		if(compositor_status()) composite();
		uint32_t total_intensity = driver_hub75_render(brightness, hub75_framebuffer);

#ifdef CONFIG_HUB75_REGULATE_VLED
                if (xTaskGetTickCount() - lastUsbCheck > 1000/portTICK_PERIOD_MS) {
                    lastUsbCheck = xTaskGetTickCount();
                    int vusb = adc1_get_raw(ADC1_CHANNEL_6) * 2; // in raw dimensionless units
//                    printf("vusb: %d\n", vusb);
                    usb_connected = vusb > 1000; // arbitrary cutoff (normal values 1500-5000 when connected)
                }

                if (usb_connected) {
                    deduced_current = 35 * total_intensity/(5012*8); // in milliamps

                    if (deduced_current <= LOWER) {
                      duty = PCT_MIN;
                    } else if (deduced_current <= MID) {
                      duty = (int)((deduced_current - LOWER) / ((float)MID - LOWER) *
                                   (PCT_MID - PCT_MIN)) +
                             PCT_MIN;
                    } else if (deduced_current <= UPPER) {
                      duty = (int)((deduced_current - MID) / ((float)UPPER - MID) * (255 - PCT_MID)) +
                             PCT_MID;
                    }
                } else {
                  duty = 255;
                }

                if (duty != 255) {
//                    printf("%d %d\n", deduced_current, duty);
                }
                ledc_set_duty(led_power_dimmer.speed_mode, led_power_dimmer.channel, duty);
                ledc_update_duty(led_power_dimmer.speed_mode, led_power_dimmer.channel);
#endif // CONFIG_HUB75_REGULATE_VLED

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

#ifdef CONFIG_HUB75_REGULATE_VLED
        // Initialise the vUSB sensor so we can sense when vLED correction is needed
        adc1_config_width(ADC_WIDTH_BIT_10);
        adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
#endif

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
