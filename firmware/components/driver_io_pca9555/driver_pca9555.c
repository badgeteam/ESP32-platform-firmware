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
uint8_t reg_config[2]   = {0xFF, 0xFF};
uint8_t reg_polarity[2] = {0xFF, 0xFF};
uint8_t reg_output[2]   = {0xFF, 0xFF};

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

bool pin_configured_as_always_output(int pin) {
	#ifdef CONFIG_DRIVER_PCA9555_IO0P0_OUTPUT
		if (pin == 0) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P1_OUTPUT
		if (pin == 1) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P2_OUTPUT
		if (pin == 2) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P3_OUTPUT
		if (pin == 3) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P4_OUTPUT
		if (pin == 4) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P5_OUTPUT
		if (pin == 5) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P6_OUTPUT
		if (pin == 6) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P7_OUTPUT
		if (pin == 7) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P0_OUTPUT
		if (pin == 8) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P1_OUTPUT
		if (pin == 9) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P2_OUTPUT
		if (pin == 10) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P3_OUTPUT
		if (pin == 11) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P4_OUTPUT
		if (pin == 12) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P5_OUTPUT
		if (pin == 13) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P6_OUTPUT
		if (pin == 14) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P7_OUTPUT
		if (pin == 15) return true;
	#endif
	return false;
}

bool pin_configured_as_always_input(int pin) {
	#ifdef CONFIG_DRIVER_PCA9555_IO0P0_INPUT
		if (pin == 0) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P1_INPUT
		if (pin == 1) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P2_INPUT
		if (pin == 2) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P3_INPUT
		if (pin == 3) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P4_INPUT
		if (pin == 4) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P5_INPUT
		if (pin == 5) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P6_INPUT
		if (pin == 6) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO0P7_INPUT
		if (pin == 7) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P0_INPUT
		if (pin == 8) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P1_INPUT
		if (pin == 9) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P2_INPUT
		if (pin == 10) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P3_INPUT
		if (pin == 11) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P4_INPUT
		if (pin == 12) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P5_INPUT
		if (pin == 13) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P6_INPUT
		if (pin == 14) return true;
	#endif
	#ifdef CONFIG_DRIVER_PCA9555_IO1P7_INPUT
		if (pin == 15) return true;
	#endif
	return false;
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
	reg_config[0] = 0xFF; // By default set all pins to input
	reg_config[1] = 0xFF;

	for (uint8_t i = 0; i < 8; i++) {
		if (pin_configured_as_always_output(i))   reg_config[0] &= ~(1 << i);
		if (pin_configured_as_always_output(i+8)) reg_config[1] &= ~(1 << i);
	}
	
	res = driver_pca9555_write_reg(PCA9555_REG_CONFIG_0, reg_config, 2); //Writes port mode to both config 0 and config 1 registers
	if (res != ESP_OK) return res;
	
	reg_polarity[0] = CONFIG_DRIVER_PCA9555_POLARITY_0;
	reg_polarity[1] = CONFIG_DRIVER_PCA9555_POLARITY_1;
	res = driver_pca9555_write_reg(PCA9555_REG_POLARITY_0, reg_polarity, 2); //Writes port polarity to both port 0 and port 1 registers
	if (res != ESP_OK) return res;
	
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

int driver_pca9555_set_gpio_direction(int pin, bool direction)
{
	if ((pin < 0) || (pin > 15)) return -1; //Out of range
	if (pin_configured_as_always_output(pin)) return -1; //Not allowed to change direction, always output
	if (pin_configured_as_always_input(pin))  return -1; //Not allowed to change direction, always input
	uint8_t port = (pin >= 8) ? 1 : 0;
	uint8_t bit  = pin % 8;
	bool current_state = ((reg_config[port] >> bit) & 1) ? 0 : 1;
	if (direction != current_state) {
		if (direction) {
			reg_config[port] &= ~(1 << bit); //Set the pin to output
		} else {
			reg_config[port] |= (1 << bit); //Set the pin to input
		}
	}
	esp_err_t res = driver_pca9555_write_reg(PCA9555_REG_CONFIG_0, reg_config, 2); 
	if (res != ESP_OK) return -1;
	return 0;
}

int driver_pca9555_get_gpio_direction(int pin)
{
	if ((pin < 0) || (pin > 15)) return -1; //Out of range
	uint8_t port = (pin >= 8) ? 1 : 0;
	uint8_t bit  = pin % 8;
	return ((reg_config[port] >> bit) & 1) ? 0 : 1; //Return 0 when the pin is an input and 1 when the pin is output
}

int driver_pca9555_set_gpio_polarity(int pin, bool polarity)
{
	if ((pin < 0) || (pin > 15)) return -1; //Out of range
	if (pin_configured_as_always_output(pin)) return -1; //Not allowed to change polarity
	if (pin_configured_as_always_input(pin))  return -1; //Not allowed to change polarity
	uint8_t port = (pin >= 8) ? 1 : 0;
	uint8_t bit  = pin % 8;
	bool current_state = ((reg_polarity[port] >> bit) & 1) ? 0 : 1;
	if (polarity != current_state) {
		if (polarity) {
			reg_polarity[port] &= ~(1 << bit); //Set the pin to output
		} else {
			reg_polarity[port] |= (1 << bit); //Set the pin to input
		}
	}
	esp_err_t res = driver_pca9555_write_reg(PCA9555_REG_POLARITY_0, reg_polarity, 2); 
	if (res != ESP_OK) return -1;
	return 0;
}

int driver_pca9555_get_gpio_polarity(int pin)
{
	if ((pin < 0) || (pin > 15)) return -1; //Out of range
	uint8_t port = (pin >= 8) ? 1 : 0;
	uint8_t bit  = pin % 8;
	return ((reg_polarity[port] >> bit) & 1) ? 0 : 1; //Return 0 when the pin is in normal mode and 1 when the pin is in inverted mode
}

int driver_pca9555_set_gpio_value(int pin, bool value)
{
	if ((pin < 0) || (pin > 15)) return -1; //Out of range
	uint8_t port = (pin >= 8) ? 1 : 0;
	uint8_t bit  = pin % 8;
	if (!driver_pca9555_get_gpio_direction(pin)) return -1; // Pin is an input
	if (value) {
		reg_output[port] |= (1 << bit);
	} else {
		reg_output[port] &= ~(1 << bit);
	}
	uint8_t reg = port ? PCA9555_REG_OUTPUT_1 : PCA9555_REG_OUTPUT_0;
	esp_err_t res = driver_pca9555_write_reg(reg, &reg_output[port], 1); 
	if (res != ESP_OK) return -1;
	return 0;
}

int driver_pca9555_get_gpio_value(int pin)
{
	if ((pin < 0) || (pin > 15)) return -1; //Out of range
	uint8_t port = (pin >= 8) ? 1 : 0;
	uint8_t bit  = pin % 8;
	uint8_t reg;
	if (driver_pca9555_get_gpio_direction(pin)) {
		reg = port ? PCA9555_REG_OUTPUT_1 : PCA9555_REG_OUTPUT_0;
	} else {
		reg = port ? PCA9555_REG_INPUT_1 : PCA9555_REG_INPUT_0;
	}
	uint8_t reg_value;
	esp_err_t res = driver_pca9555_read_reg(reg, &reg_value, 1);
	if (res != ESP_OK) return -1;
	return (reg_value>>bit)&1;
}

#else
esp_err_t driver_pca9555_init(void) {return ESP_OK;} //Dummy function, leave empty.
#endif
