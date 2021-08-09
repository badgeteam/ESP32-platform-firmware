#include <sdkconfig.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <soc/gpio_reg.h>
#include <soc/gpio_sig_map.h>
#include <soc/gpio_struct.h>
#include <soc/spi_reg.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>

#include "include/driver_mch2021_stm32.h"
#include <driver_pca9555.h>

#ifdef CONFIG_DRIVER_MCH2021_STM32_ENABLE

#define MESSAGE_SIZE 18

static spi_device_handle_t spiDevice = NULL;
static uint8_t receive_buffer[MESSAGE_SIZE];

esp_err_t driver_mch2021_stm32_transaction(const uint8_t* out, uint8_t* in) {
    spi_transaction_t t = {
        .length = MESSAGE_SIZE * 8,  // transaction length is in bits
        .tx_buffer = out,
        .rx_buffer = in
    };
    return spi_device_transmit(spiDevice, &t);
}

esp_err_t driver_mch2021_stm32_reset(bool mode) {
    esp_err_t res = ESP_FAIL;
    #ifdef CONFIG_PIN_NUM_MCH2021_STM32_RESET
        res = gpio_set_level(CONFIG_PIN_NUM_MCH2021_STM32_RESET, false);
    #endif
    #ifdef CONFIG_PIN_NUM_MCH2021_STM32_RESET_PCA9555
        res = (driver_pca9555_set_gpio_value(CONFIG_PIN_NUM_MCH2021_STM32_RESET_PCA9555, false)==0) ? ESP_OK : ESP_FAIL;
    #endif
    #ifdef CONFIG_PIN_NUM_MCH2021_STM32_BOOT0
        res = gpio_set_level(CONFIG_PIN_NUM_MCH2021_STM32_BOOT0, mode);
    #endif
    #ifdef CONFIG_PIN_NUM_MCH2021_STM32_BOOT0_PCA9555
        res = (driver_pca9555_set_gpio_value(CONFIG_PIN_NUM_MCH2021_STM32_BOOT0_PCA9555, mode)==0) ? ESP_OK : ESP_FAIL;
    #endif
    vTaskDelay(100 / portTICK_PERIOD_MS);
    #ifdef CONFIG_PIN_NUM_MCH2021_STM32_RESET
        res = gpio_set_level(CONFIG_PIN_NUM_MCH2021_STM32_RESET, true);
    #endif
    #ifdef CONFIG_PIN_NUM_MCH2021_STM32_RESET_PCA9555
        res = (driver_pca9555_set_gpio_value(CONFIG_PIN_NUM_MCH2021_STM32_RESET_PCA9555, true)==0) ? ESP_OK : ESP_FAIL;
    #endif
    return res;
}

esp_err_t driver_mch2021_stm32_init(void) {
    esp_err_t res;
        
    //Initialize reset GPIO pin
    #ifdef CONFIG_PIN_NUM_MCH2021_STM32_RESET
        res = gpio_set_direction(CONFIG_PIN_NUM_MCH2021_STM32_RESET, GPIO_MODE_OUTPUT);
        if (res != ESP_OK) return res;
        res = gpio_set_level(CONFIG_PIN_NUM_MCH2021_STM32_RESET, true);
        if (res != ESP_OK) return res;
    #endif
        
    #ifdef CONFIG_PIN_NUM_MCH2021_STM32_RESET_PCA9555
        res = (driver_pca9555_set_gpio_value(CONFIG_PIN_NUM_MCH2021_STM32_RESET_PCA9555, true)==0) ? ESP_OK : ESP_FAIL;
        if (res != ESP_OK) return res;
    #endif
    
    //Initialize boot0 GPIO pin
    #ifdef CONFIG_PIN_NUM_MCH2021_STM32_BOOT0
        res = gpio_set_direction(CONFIG_PIN_NUM_MCH2021_STM32_BOOT0, GPIO_MODE_INPUT);
        if (res != ESP_OK) return res;
        res = gpio_set_level(CONFIG_PIN_NUM_MCH2021_STM32_BOOT0, false);
        if (res != ESP_OK) return res;
    #endif

    #ifdef CONFIG_PIN_NUM_MCH2021_STM32_BOOT0_PCA9555
        res = (driver_pca9555_set_gpio_value(CONFIG_PIN_NUM_MCH2021_STM32_BOOT0_PCA9555, false)==0) ? ESP_OK : ESP_FAIL;
        if (res != ESP_OK) return res;
    #endif
        
    //Initialize interrupt GPIO pin
    #if CONFIG_PIN_NUM_MCH2021_STM32_INT >= 0
        res = gpio_set_direction(CONFIG_PIN_NUM_MCH2021_STM32_INT, GPIO_MODE_INPUT);
        if (res != ESP_OK) return res;
    #endif
        
    static const spi_device_interface_config_t devcfg = {
        .clock_speed_hz = CONFIG_DRIVER_MCH2021_STM32_SPI_SPEED,
        .mode           = 0,  // SPI mode 0
        .spics_io_num   = CONFIG_PIN_NUM_MCH2021_STM32_CS,
        .queue_size     = 1,
        .flags          = 0,
    };
    res = spi_bus_add_device(VSPI_HOST, &devcfg, &spiDevice);
    
    return res;
}

esp_err_t driver_mch2021_stm32_send_command(uint16_t command, uint8_t* parameters, uint8_t num_parameters) {
    if (num_parameters > MESSAGE_SIZE - 2) return ESP_FAIL;
    uint8_t transmit_buffer[MESSAGE_SIZE];
    memset(transmit_buffer, 0, sizeof(transmit_buffer));
    transmit_buffer[0] = command & 0xFF;
    transmit_buffer[1] = (command >> 8) & 0xFF;
    for (uint8_t i = 0; i < num_parameters; i++) {
        transmit_buffer[i + 2] = parameters[i];
    }
    return driver_mch2021_stm32_transaction(transmit_buffer, receive_buffer);
}

esp_err_t driver_mch2021_stm32_lcd_set_mode(bool mode) {
    uint8_t mode_byte = mode;
    return driver_mch2021_stm32_send_command(1, &mode_byte, 1);
}

esp_err_t driver_mch2021_stm32_lcd_set_backlight(uint8_t brightness) {
    return driver_mch2021_stm32_send_command(2, &brightness, 1);
}

#else
esp_err_t driver_mch2021_stm32_init(void) { return ESP_OK; } //Dummy function.
#endif
