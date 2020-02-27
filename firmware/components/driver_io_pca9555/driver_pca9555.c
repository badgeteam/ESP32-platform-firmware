#include <sdkconfig.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <nvs.h>
#include <driver/gpio.h>
#include <driver_i2c.h>
#include "include/driver_pca9555.h"

#ifdef CONFIG_DRIVER_PCA9555_ENABLE

static const char *TAG = "pca9555";

xSemaphoreHandle driver_pca9555_mux = NULL;          // mutex for accessing driver_pca9555_state, driver_pca9555_handlers, etc..
xSemaphoreHandle driver_pca9555_intr_trigger = NULL; // semaphore to trigger PCA95XX interrupt handling
driver_pca9555_intr_t driver_pca9555_handler[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }; // Per port interrupt handler
TaskHandle_t driver_pca9555_intr_task_handle = NULL;

/* I2C access */

static inline esp_err_t driver_pca9555_read_reg(uint8_t reg, uint8_t *data, size_t data_len)
{
	esp_err_t res = driver_i2c_read_reg(CONFIG_I2C_ADDR_PCA9555, reg, data, data_len);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c pca9555 read reg error %d", res);
	}
	return res;
}

static inline esp_err_t driver_pca9555_write_reg(uint8_t reg, uint8_t *data, size_t data_len)
{
	esp_err_t res = driver_i2c_write_reg_n(CONFIG_I2C_ADDR_PCA9555, reg, data, data_len);
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c pca9555 write reg error %d", res);
	}
	return res;
}

/* Interrupt handling */

void driver_pca9555_intr_task(void *arg)
{
	esp_err_t res;
	uint16_t previous_state = 0;
	uint8_t data[] = {0,0};
	
	while (1) {
		if (xSemaphoreTake(driver_pca9555_intr_trigger, portMAX_DELAY)) {
			res = driver_pca9555_read_reg(PCA9555_REG_INPUT_0, data, 2);
			if (res != ESP_OK) {
				ESP_LOGE(TAG, "pca9555: failed to read input state");
			}
			uint16_t current_state = data[0] + (data[1]<<8);
			for (int i = 0; i < 15; i++) {
				if ((current_state & (1 << i)) != (previous_state & (1 << i))) {
					bool value = (current_state & (1 << i)) > 0;
					xSemaphoreTake(driver_pca9555_mux, portMAX_DELAY);
					driver_pca9555_intr_t handler = driver_pca9555_handler[i];
					xSemaphoreGive(driver_pca9555_mux);
					if (handler != NULL) handler(i, value);
					printf("PCA9555 PIN CHANGE %d = %d\n", i, value);
				}
			}
			vTaskDelay(10 / portTICK_PERIOD_MS);
			previous_state = current_state;
		}
	}
}

void driver_pca9555_intr_handler(void *arg)
{ /* in interrupt handler */
	xSemaphoreGiveFromISR(driver_pca9555_intr_trigger, NULL);
}

/* Public functions */

void driver_pca9555_set_interrupt_handler(uint8_t pin, driver_pca9555_intr_t handler)
{
	if (pin >= 16) return;
	if (driver_pca9555_mux == NULL) { // allow setting handlers when driver_pca9555 is not initialized yet.
		driver_pca9555_handler[pin] = handler;
	} else {
		xSemaphoreTake(driver_pca9555_mux, portMAX_DELAY);
		driver_pca9555_handler[pin] = handler;
		xSemaphoreGive(driver_pca9555_mux);
	}
}

esp_err_t driver_pca9555_init(void)
{
	static bool driver_pca9555_init_done = false;
	if (driver_pca9555_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	esp_err_t res;
	
	//Create mutex
	driver_pca9555_mux = xSemaphoreCreateMutex();
	if (driver_pca9555_mux == NULL) return ESP_ERR_NO_MEM;
	
	//Create interrupt trigger
	driver_pca9555_intr_trigger = xSemaphoreCreateBinary();
	if (driver_pca9555_intr_trigger == NULL) return ESP_ERR_NO_MEM;
	
	//Configure pins
	uint8_t config[] = {0xFF,0xFF}; // By default set all pins to input
	#ifdef CONFIG_DRIVER_PCA9555_IO0P0_OUTPUT
		config[0] &= ~(1 << 0);
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P1_OUTPUT
		config[0] &= ~(1 << 1);
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P2_OUTPUT
		config[0] &= ~(1 << 2);
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P3_OUTPUT
		config[0] &= ~(1 << 3);
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P4_OUTPUT
		config[0] &= ~(1 << 4);
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P5_OUTPUT
		config[0] &= ~(1 << 5);
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P6_OUTPUT
		config[0] &= ~(1 << 6);
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P7_OUTPUT
		config[0] &= ~(1 << 7);
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P0_OUTPUT
		config[1] &= ~(1 << 0);
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P1_OUTPUT
		config[1] &= ~(1 << 1);
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P2_OUTPUT
		config[1] &= ~(1 << 2);
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P3_OUTPUT
		config[1] &= ~(1 << 3);
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P4_OUTPUT
		config[1] &= ~(1 << 4);
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P5_OUTPUT
		config[1] &= ~(1 << 5);
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P6_OUTPUT
		config[1] &= ~(1 << 6);
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P7_OUTPUT
		config[1] &= ~(1 << 7);
	#endif
	printf("PCA9555 port mode: %02X, %02X\n", config[0], config[1]);
	res = driver_pca9555_write_reg(PCA9555_REG_CONFIG_0, config, 2); //Writes port mode to both config 0 and config 1 registers
	//if (res != ESP_OK) return res;
	
	config[0] = CONFIG_DRIVER_PCA9555_POLARITY_0;
	config[1] = CONFIG_DRIVER_PCA9555_POLARITY_1;
	res = driver_pca9555_write_reg(PCA9555_REG_POLARITY_0, config, 2); //Writes port polarity to both port 0 and port 1 registers
	//if (res != ESP_OK) return res;
	
	//Attach interrupt to interrupt pin (if available)
	#if CONFIG_PIN_NUM_PCA9555_INT >= 0
		res = gpio_isr_handler_add(CONFIG_PIN_NUM_PCA9555_INT, driver_pca9555_intr_handler, NULL);
		if (res != ESP_OK) return res;

		gpio_config_t io_conf = {
			.intr_type    = GPIO_INTR_NEGEDGE,
			.mode         = GPIO_MODE_INPUT,
			.pin_bit_mask = 1LL << CONFIG_PIN_NUM_PCA9555_INT,
			.pull_down_en = 0,
			.pull_up_en   = 1,
		};

		res = gpio_config(&io_conf);
		if (res != ESP_OK) return res;
		
		xTaskCreate(&driver_pca9555_intr_task, "PCA9555 interrupt task", 4096, NULL, 10, &driver_pca9555_intr_task_handle);
		xSemaphoreGive(driver_pca9555_intr_trigger);
	#endif
	
	driver_pca9555_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

#else
esp_err_t driver_pca9555_init(void) {return ESP_OK;} //Dummy function, leave empty.
#endif
