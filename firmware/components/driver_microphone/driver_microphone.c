#include <sdkconfig.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <esp_err.h>

#include "include/driver_microphone.h"

#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/i2s.h"
#include "esp_sleep.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/dac.h"
#include "soc/rtc_cntl_reg.h"

#ifdef CONFIG_DRIVER_MICROPHONE_ENABLE

#define TAG "microphone"

//#define CONFIG_DRIVER_MICROPHONE_I2S_NUM 1

#define READ_LEN (2 * 1024)

uint8_t BUFFER[READ_LEN] = {0};


void ICS41350_disp_buf(uint8_t* buf, int length)
{
    printf("======\n");
    for (int i = 0; i < length; i++) {
        printf("%02x ", buf[i]);
        if ((i + 1) % 8 == 0) {
            printf("\n");
        }
    }
    printf("======\n");
}

void ICS41350_record_task (void* arg)
{
    while (1)
	{
	    i2s_read_bytes(CONFIG_DRIVER_MICROPHONE_I2S_NUM, (char*) BUFFER, READ_LEN, (1000 / portTICK_RATE_MS));
	    ICS41350_disp_buf((uint8_t*) BUFFER, READ_LEN);
	    vTaskDelay(1000 / portTICK_RATE_MS);
	}

}

esp_err_t driver_microphone_test(void)
{
	xTaskCreate(ICS41350_record_task, "ICS41350_record_task", 1024 * 2, NULL, 5, NULL);
	return ESP_OK;
}

esp_err_t driver_microphone_init(void)
{
	static bool driver_microphone_init_done = false;
	if (driver_microphone_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	i2s_config_t i2s_config = {
		.mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM,
		.sample_rate =  44100,
		.bits_per_sample = 16,
		.communication_format = I2S_COMM_FORMAT_PCM,
		.channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
		.intr_alloc_flags = 0,
		.dma_buf_count = 2,
		.dma_buf_len = 1024
	};

	i2s_pin_config_t pin_config = {
		.ws_io_num   = 25,
		.data_in_num = 35,
	};

	//install and start i2s driver
	i2s_driver_install(CONFIG_DRIVER_MICROPHONE_I2S_NUM, &i2s_config, 0, NULL);
	i2s_set_pin(CONFIG_DRIVER_MICROPHONE_I2S_NUM, &pin_config);
	i2s_set_clk(CONFIG_DRIVER_MICROPHONE_I2S_NUM, 44100, 16, I2S_CHANNEL_MONO);
	
	driver_microphone_test();
	
	driver_microphone_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

#else // CONFIG_DRIVER_MICROPHONE_ENABLE
esp_err_t driver_microphone_init(void) { return ESP_OK; } // Dummy function, leave empty!
#endif
