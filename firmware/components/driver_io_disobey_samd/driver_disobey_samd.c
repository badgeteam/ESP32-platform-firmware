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

#include "driver_i2c.h"
#include "include/driver_disobey_samd.h"

#ifdef CONFIG_DRIVER_DISOBEY_SAMD_ENABLE

static const char *TAG = "Disobey I/O";

// mutex for accessing driver_disobey_samd_state, driver_disobey_samd_handlers, etc..
xSemaphoreHandle driver_disobey_samd_mux = NULL;

// semaphore to trigger disobey_samd interrupt handling
xSemaphoreHandle driver_disobey_samd_intr_trigger = NULL;

driver_disobey_samd_intr_t driver_disobey_samd_handler = NULL;

int driver_disobey_samd_read_state()
{
	uint8_t state[2];
	esp_err_t res = driver_i2c_read_reg(CONFIG_I2C_ADDR_DISOBEY_SAMD, 0, state, 2);

	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c read state error %d", res);
		return -1;
	}

	int value = state[0] + (state[1] << 8);

	ESP_LOGD(TAG, "i2c read state 0x%04x", value);

	return value;
}

int driver_disobey_samd_read_touch()
{
	int touch = driver_disobey_samd_read_state() & 0x3F;
	//ets_printf("driver_disobey_samd: read touch: 0x%02X\n", touch);
	return touch;
}

int driver_disobey_samd_read_usb()
{
	return (driver_disobey_samd_read_state()>>6) & 0x01;
}

int driver_disobey_samd_read_battery()
{
	return driver_disobey_samd_read_state()>>8 & 0xFF;
}

static inline esp_err_t driver_disobey_samd_write_reg(uint8_t reg, uint8_t value)
{
	esp_err_t res = driver_i2c_write_reg(CONFIG_I2C_ADDR_DISOBEY_SAMD, reg, value);

	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write reg(0x%02x, 0x%02x): error %d", reg, value, res);
		return res;
	}

	ESP_LOGD(TAG, "i2c write reg(0x%02x, 0x%02x): ok", reg, value);

	return res;
}

static inline esp_err_t driver_disobey_samd_write_reg32(uint8_t reg, uint32_t value)
{
	esp_err_t res = driver_i2c_write_reg32(CONFIG_I2C_ADDR_DISOBEY_SAMD, reg, value);

	if (res != ESP_OK) {
		ESP_LOGE(TAG, "i2c write reg32(0x%08x, 0x%02x): error %d", reg, value, res);
		return res;
	}

	ESP_LOGD(TAG, "i2c write reg32(0x%02x, 0x%08x): ok", reg, value);

	return res;
}

esp_err_t driver_disobey_samd_write_backlight(uint8_t value)
{
	return driver_disobey_samd_write_reg(disobey_samd_CMD_BACKLIGHT, value);
}

esp_err_t driver_disobey_samd_write_led(uint8_t led, uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t value = led + (r << 8) + (g << 16) + (b << 24);
	return driver_disobey_samd_write_reg32(disobey_samd_CMD_LED, value);
}

esp_err_t driver_disobey_samd_write_buzzer(uint16_t freqency, uint16_t duration)
{
	uint32_t value = freqency + (duration << 16);
	return driver_disobey_samd_write_reg32(disobey_samd_CMD_BUZZER, value);
}

esp_err_t driver_disobey_samd_write_off()
{
	return driver_disobey_samd_write_reg32(disobey_samd_CMD_OFF, 0);
}

void driver_disobey_samd_intr_task(void *arg)
{
	// we cannot use I2C in the interrupt handler, so we
	// create an extra thread for this..

	int old_state = 0;
	while (1)
	{
		if (xSemaphoreTake(driver_disobey_samd_intr_trigger, portMAX_DELAY))
		{
			int state;
			while (1)
			{
				state = driver_disobey_samd_read_touch();
				if (state != -1)
					break;

				ESP_LOGE(TAG, "failed to read status.");
				vTaskDelay(1000 / portTICK_PERIOD_MS);
			}

			int pressed = 0;
			int released = 0;

			for (uint8_t i = 0; i<6; i++) {
				if (((state>>i)&0x01)&&(!((old_state>>i)&0x01))) pressed |= (1<<i);
				if (((old_state>>i)&0x01)&&(!((state>>i)&0x01))) released |= (1<<i);
			}

			if (state != old_state) {
				xSemaphoreTake(driver_disobey_samd_mux, portMAX_DELAY);
				driver_disobey_samd_intr_t handler = driver_disobey_samd_handler;
				xSemaphoreGive(driver_disobey_samd_mux);

				if (handler != NULL) {
					handler(pressed, released);
				}
			}

			old_state = state;
		}
	}
}

void driver_disobey_samd_intr_handler(void *arg)
{ /* in interrupt handler */
	int gpio_state = gpio_get_level(CONFIG_PIN_NUM_DISOBEY_SAMD_INT);

#ifdef CONFIG_DRIVER_DISOBEY_SAMD_DEBUG
	static int gpio_last_state = -1;
	if (gpio_state != -1 && gpio_last_state != gpio_state)
	{
		ets_printf("driver_disobey_samd: I2C Int %s\n", gpio_state == 0 ? "up" : "down");
	}
	gpio_last_state = gpio_state;
#endif // CONFIG_SHA_DRIVER_disobey_samd_DEBUG

	if (gpio_state == 0) {
		xSemaphoreGiveFromISR(driver_disobey_samd_intr_trigger, NULL);
	}
}

esp_err_t driver_disobey_samd_init(void)
{
	static bool driver_disobey_samd_init_done = false;
	if (driver_disobey_samd_init_done)return ESP_OK;
	ESP_LOGD(TAG, "init called");

	esp_err_t res = driver_i2c_init();
	if (res != ESP_OK) return res;

	driver_disobey_samd_mux = xSemaphoreCreateMutex();
	if (driver_disobey_samd_mux == NULL) return ESP_ERR_NO_MEM;

	driver_disobey_samd_intr_trigger = xSemaphoreCreateBinary();
	if (driver_disobey_samd_intr_trigger == NULL) return ESP_ERR_NO_MEM;

	res = gpio_isr_handler_add(CONFIG_PIN_NUM_DISOBEY_SAMD_INT, driver_disobey_samd_intr_handler, NULL);
	if (res != ESP_OK) return res;

	gpio_config_t io_conf = {
		.intr_type    = GPIO_INTR_ANYEDGE,
		.mode         = GPIO_MODE_INPUT,
		.pin_bit_mask = 1LL << CONFIG_PIN_NUM_DISOBEY_SAMD_INT,
		.pull_down_en = 0,
		.pull_up_en   = 0,
	};
	res = gpio_config(&io_conf);
	if (res != ESP_OK) return res;
	xTaskCreate(&driver_disobey_samd_intr_task, "disobey_samd interrupt task", 4096, NULL, 10, NULL);
	xSemaphoreGive(driver_disobey_samd_intr_trigger);
	driver_disobey_samd_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

void driver_disobey_samd_set_interrupt_handler(driver_disobey_samd_intr_t handler)
{
	if (driver_disobey_samd_mux == NULL)
	{ // allow setting handler when driver_disobey_samd is not initialized yet.
		driver_disobey_samd_handler = handler;
	} else {
		xSemaphoreTake(driver_disobey_samd_mux, portMAX_DELAY);
		driver_disobey_samd_handler = handler;
		xSemaphoreGive(driver_disobey_samd_mux);
	}
}

esp_err_t driver_disobey_samd_get_touch_info(struct driver_disobey_samd_touch_info *info)
{
	int value = driver_disobey_samd_read_touch();
	if (value == -1) return ESP_FAIL; // need more-specific error?
	info->touch_state = value;
	return ESP_OK;
}

#else

esp_err_t driver_disobey_samd_init(void) {
    return ESP_OK;
}

#endif // CONFIG_DRIVER_DISOBEY_SAMD_ENABLE
