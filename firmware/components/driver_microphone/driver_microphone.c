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

#define READ_LEN (1024*64)

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

static QueueHandle_t soundQueue;
void init_output()
{
	i2s_config_t cfg={
		.mode=I2S_MODE_TX|I2S_MODE_MASTER,
		.sample_rate= 44100,
		.bits_per_sample=16,
		.channel_format=I2S_CHANNEL_FMT_RIGHT_LEFT,
		.communication_format=I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_LSB,
		.intr_alloc_flags=0,
		.dma_buf_count=4,
		.dma_buf_len=1024
	};
	
	static const i2s_pin_config_t pin_config = {
		.bck_io_num = 13,
		.ws_io_num = 21,
		.data_out_num = 18,
		.data_in_num = I2S_PIN_NO_CHANGE
	};
	
	i2s_driver_install(1, &cfg, 4, &soundQueue);
	i2s_set_sample_rates(1, cfg.sample_rate);
	i2s_set_pin(1, &pin_config);
}

void ICS41350_record_task (void* arg)
{
	uint8_t *buffer = malloc(READ_LEN);
	if (!buffer) {
		ESP_LOGE(TAG, "MALLOC FAILED");
		return;
	}
	
	init_output();
	
    while (1)
	{
	    int read = i2s_read_bytes(CONFIG_DRIVER_MICROPHONE_I2S_NUM, (char*) buffer, READ_LEN, 1);
	    //ICS41350_disp_buf((uint8_t*) buffer, READ_LEN);
	    //vTaskDelay(1000 / portTICK_RATE_MS);
		i2s_write_bytes(1, (char*) buffer, read, portMAX_DELAY);
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
		.sample_rate = 44100,
		.bits_per_sample = 16,
		.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
		.communication_format = I2S_COMM_FORMAT_PCM,
		.dma_buf_count = 2,
		.dma_buf_len = 8,
		.use_apll = 0,
		.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1
	};

	i2s_pin_config_t pin_config = {
		.ws_io_num   = 25,
		.data_in_num = 35,
	};

	//install and start i2s driver
	i2s_driver_install(CONFIG_DRIVER_MICROPHONE_I2S_NUM, &i2s_config, 0, NULL);
	i2s_set_pin(CONFIG_DRIVER_MICROPHONE_I2S_NUM, &pin_config);
	//i2s_set_clk(CONFIG_DRIVER_MICROPHONE_I2S_NUM, 44100, 16, I2S_CHANNEL_MONO);
	
	driver_microphone_test();
	
	driver_microphone_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

#else // CONFIG_DRIVER_MICROPHONE_ENABLE
esp_err_t driver_microphone_init(void) { return ESP_OK; } // Dummy function, leave empty!
#endif
