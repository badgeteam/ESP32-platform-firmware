#include <sdkconfig.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>

#include "driver/touch_pad.h"
#include "include/driver_touchbuttons.h"

#ifdef CONFIG_DRIVER_IO_TOUCHBUTTONS_ENABLE

#define TAG "io_touchbuttons"

/****** Touch pad mapping *******
 * 0: GPIO 4
 * 1: GPIO 0
 * 2: GPIO 2
 * 3: GPIO 15
 * 4: GPIO 13
 * 5: GPIO 12
 * 6: GPIO 14
 * 7: GPIO 27
 * 8: GPIO 33
 * 9: GPIO 32
 */

static bool enabled[] = {
    CONFIG_TOUCH0_ENABLED,
    CONFIG_TOUCH1_ENABLED,
    CONFIG_TOUCH2_ENABLED,
    CONFIG_TOUCH3_ENABLED,
    CONFIG_TOUCH4_ENABLED,
    CONFIG_TOUCH5_ENABLED,
    CONFIG_TOUCH6_ENABLED,
    CONFIG_TOUCH7_ENABLED,
    CONFIG_TOUCH8_ENABLED,
    CONFIG_TOUCH9_ENABLED
};
static uint16_t prev_touch_state = 0, touch_state = 0;
static uint32_t initial_values[TOUCH_PAD_MAX];

/*
  Read values sensed at all available touch pads.
  Use 2 / 3 of read value as the threshold
  to trigger interrupt when the pad is touched.
  Note: this routine demonstrates a simple way
  to configure activation threshold for the touch pads.
  Do not touch any pads when this routine
  is running (on application start).
 */
static void set_thresholds(void)
{
    uint16_t touch_value;
    for (int i = 0; i<TOUCH_PAD_MAX; i++) {
        if(!enabled[i]) { continue; }

        //read filtered value
        touch_pad_read(i, &touch_value);
        initial_values[i] = touch_value;
        ESP_LOGI(TAG, "test init: touch pad [%d] val is %d", i, touch_value);
        //set interrupt threshold.
        ESP_ERROR_CHECK(touch_pad_set_thresh(i, touch_value * 2 / 3));

    }
}

/*
 * Before reading touch pad, we need to initialize the RTC IO.
 */
static void config_touch_pads()
{
    for (int i = 0;i< TOUCH_PAD_MAX;i++) {
        if(!enabled[i]) { continue; }

        // Init RTC IO and mode for touch pad, but don't enable them yet
        // (threshold = 0, will be set in set_thresholds()).
        touch_pad_config(i, 0);
    }
}

static void (*touch_handler)(uint16_t) = NULL;
esp_err_t set_touch_handler(void (*handler)(uint16_t))
{
    touch_handler = handler;
    return ESP_OK;
}

uint16_t get_touch_state() {
    return prev_touch_state;
}

uint16_t get_touch_value(uint8_t pad) {
    uint16_t value;
    touch_pad_read_filtered(pad, &value);
    return value;
}

static bool run = true;
IRAM_ATTR void triggered() {
    uint16_t value;
    bool touched;
    while(run) {
        touch_state = 0;
        for (int i = 0;i<TOUCH_PAD_MAX;i++) {
            if(!enabled[i]) { continue; }

            touch_pad_read(i, &value);

            touched = value <= (initial_values[i] / 3);
            touch_state |= ((uint8_t) touched) << i;
        }
        if(prev_touch_state != touch_state) {
            prev_touch_state = touch_state;

            if (touch_handler != NULL) {
                touch_handler(touch_state);
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

static bool initialised = false;
esp_err_t driver_touchbuttons_init(void)
{
    // Initialize touch pad peripheral, it will start a timer to run a filter
    ESP_LOGI(TAG, "Initializing touch pad");

    // Initialize touch pad peripheral.
    // The default fsm mode is software trigger mode.
    touch_pad_init();

    // Set reference voltage for charging/discharging
    // In this case, the high reference valtage will be 2.7V - 1V = 1.7V
    // The low reference voltage will be 0.5
    // The larger the range, the larger the pulse count value.
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);

    // Enable only the pads set through config.sh
    config_touch_pads();

    set_thresholds();

#ifdef CONFIG_TOUCH_WAKEUP_SOURCE
    esp_sleep_enable_touchpad_wakeup();
#endif

    xTaskCreate( triggered, "touchbuttons", 2048, NULL, 0, NULL );

    initialised = true;
    ESP_LOGD(TAG, "init done");
    return ESP_OK;
}

#else
esp_err_t driver_touchbuttons_init(void) { return ESP_OK; } //Dummy function.
#endif // CONFIG_DRIVER_IO_TOUCHBUTTONS_ENABLE
