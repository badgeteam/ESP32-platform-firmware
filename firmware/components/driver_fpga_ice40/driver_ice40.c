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

#include "include/driver_ice40.h"
#include <driver_pca9555.h>

#ifdef CONFIG_DRIVER_ICE40_ENABLE

static spi_device_handle_t spiDevice = NULL;
static bool spiDeviceHasChipSelect = false;

esp_err_t driver_ice40_send(const uint8_t* data, int len) {
    if (len == 0) return ESP_OK;
    if (spiDevice == NULL) return ESP_FAIL;
    spi_transaction_t t = {
        .length = len * 8,  // transaction length is in bits
        .tx_buffer = data,
        .rx_buffer = NULL
    };
    return spi_device_transmit(spiDevice, &t);
}

esp_err_t driver_ice40_receive(uint8_t* data, int len) {
    if (len == 0) return ESP_OK;
    if (spiDevice == NULL) return ESP_FAIL;
    spi_transaction_t t = {
        .length = len * 8,  // transaction length is in bits
        .rxlength = len * 8,
        .tx_buffer = NULL,
        .rx_buffer = data
    };
    return spi_device_transmit(spiDevice, &t);
}

esp_err_t driver_ice40_transaction(const uint8_t* tx_data, uint8_t* rx_data, int len) {
    if (len == 0) return ESP_OK;
    if (spiDevice == NULL) return ESP_FAIL;
    spi_transaction_t t = {
        .length = len * 8,  // transaction length is in bits
        .rxlength = len * 8,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data
    };
    return spi_device_transmit(spiDevice, &t);
}

esp_err_t driver_ice40_register_device(bool enableChipSelect) {
    if (spiDeviceHasChipSelect && enableChipSelect && (spiDevice != NULL)) {
        // We're already in the correct mode
        return ESP_OK;
    }
    if ((!spiDeviceHasChipSelect) && (!enableChipSelect) && (spiDevice != NULL)) {
        // We're already in the correct mode
        return ESP_OK;
    }
    esp_err_t res = ESP_FAIL;
    if (spiDevice != NULL) {
        res = spi_bus_remove_device(spiDevice);
        spiDevice = NULL;
        if (res != ESP_OK) return res;
    }
    if (enableChipSelect) {
        static const spi_device_interface_config_t devcfg = {
            .clock_speed_hz = CONFIG_DRIVER_ICE40_SPI_SPEED_USER,
            .mode           = 0,  // SPI mode 0
            .spics_io_num   = CONFIG_PIN_NUM_ICE40_CS,
            .queue_size     = 1,
            .flags          = 0
        };
        res = spi_bus_add_device(VSPI_HOST, &devcfg, &spiDevice);
        spiDeviceHasChipSelect = true;
    } else {
        static const spi_device_interface_config_t devcfg = {
            .clock_speed_hz = CONFIG_DRIVER_ICE40_SPI_SPEED,
            .mode           = 0,  // SPI mode 0
            .spics_io_num   = -1,
            .queue_size     = 1,
            .flags          = SPI_DEVICE_HALFDUPLEX
        };
        res = spi_bus_add_device(VSPI_HOST, &devcfg, &spiDevice);
        if (res != ESP_OK) return res;
        res = gpio_set_direction(CONFIG_PIN_NUM_ICE40_CS, GPIO_MODE_OUTPUT);
        if (res != ESP_OK) return res;
        res = gpio_set_level(CONFIG_PIN_NUM_ICE40_CS, true); // Set CS pin high
        spiDeviceHasChipSelect = false;
    }
    return res;
}

esp_err_t driver_ice40_disable() {
    esp_err_t res = driver_ice40_register_device(false); // Disable the CS pin for normal SPI transfers
    if (res != ESP_OK) return res;
    #ifdef CONFIG_PIN_NUM_ICE40_RESET
        res = gpio_set_level(CONFIG_PIN_NUM_ICE40_RESET, false);
        if (res != ESP_OK) return res;
    #endif
    #ifdef CONFIG_PIN_NUM_ICE40_RESET_PCA9555
        res = (driver_pca9555_set_gpio_value(CONFIG_PIN_NUM_ICE40_RESET_PCA9555, false)==0) ? ESP_OK : ESP_FAIL;
        if (res != ESP_OK) return res;
    #endif
    if (!spiDeviceHasChipSelect) {
        res = gpio_set_level(CONFIG_PIN_NUM_ICE40_CS, true);
        if (res != ESP_OK) return res;
    }
    return ESP_OK;
}

esp_err_t driver_ice40_enable() {
    esp_err_t res = ESP_FAIL;
    #ifdef CONFIG_PIN_NUM_ICE40_RESET
        res = gpio_set_level(CONFIG_PIN_NUM_ICE40_RESET, true);
    #endif
    #ifdef CONFIG_PIN_NUM_ICE40_RESET_PCA9555
        res = (driver_pca9555_set_gpio_value(CONFIG_PIN_NUM_ICE40_RESET_PCA9555, true)==0) ? ESP_OK : ESP_FAIL;
    #endif
    return res;
}

int driver_ice40_get_done(void) {
    #ifdef CONFIG_PIN_NUM_ICE40_DONE
        return gpio_get_level(CONFIG_PIN_NUM_ICE40_DONE);
    #endif
    #ifdef CONFIG_PIN_NUM_ICE40_DONE_PCA9555
        return driver_pca9555_get_gpio_value(CONFIG_PIN_NUM_ICE40_DONE_PCA9555);
    #endif
    return 0;
}

esp_err_t driver_ice40_load_bitstream(uint8_t* bitstream, uint32_t length) {
    esp_err_t res = driver_ice40_disable(); // Put ICE40 in reset state
    if (res != ESP_OK) return res;
    res = gpio_set_level(CONFIG_PIN_NUM_ICE40_CS, false); // Set CS pin low
    if (res != ESP_OK) return res;
    res = driver_ice40_enable(); // Put ICE40 in SPI slave download state
    if (res != ESP_OK) return res;
    if (driver_ice40_get_done()) {
        printf("Before: ICE40 signals DONE (wrong)\n");
        return ESP_FAIL;
    }
    uint32_t position = 0;
    while (position < length) {
        uint32_t remaining = length - position;
        uint32_t transferSize = (remaining < CONFIG_BUS_VSPI_MAX_TRANSFERSIZE) ? remaining : CONFIG_BUS_VSPI_MAX_TRANSFERSIZE;
        driver_ice40_send(bitstream + position, transferSize);
        position += transferSize;
    }
    for (uint8_t i = 0; i < 10; i++) {
        const uint8_t dummy[10] = {0};
        driver_ice40_send(dummy, sizeof(dummy));
    }
    res = gpio_set_level(CONFIG_PIN_NUM_ICE40_CS, true); // Set CS pin high
    if (res != ESP_OK) return res;
    if (!driver_ice40_get_done()) {
        printf("After: ICE40 signals not DONE (wrong)\n");
    }
    res = driver_ice40_register_device(true); // Enable the CS pin for normal SPI transfers
    if (res != ESP_OK) return res;
    
    return ESP_OK;
}

esp_err_t driver_ice40_init(void) {	
    esp_err_t res;
        
    //Initialize reset GPIO pin
    #ifdef CONFIG_PIN_NUM_ICE40_RESET
        res = gpio_set_direction(CONFIG_PIN_NUM_ICE40_RESET, GPIO_MODE_OUTPUT);
        if (res != ESP_OK) return res;
    #endif
        
    //Initialize done GPIO pin
    #ifdef CONFIG_PIN_NUM_ICE40_DONE
        res = gpio_set_direction(CONFIG_PIN_NUM_ICE40_DONE, GPIO_MODE_INPUT);
        if (res != ESP_OK) return res;
    #endif
        
    //Initialize interrupt GPIO pin
    #if CONFIG_PIN_NUM_ICE40_INT >= 0
        res = gpio_set_direction(CONFIG_PIN_NUM_ICE40_INT, GPIO_MODE_INPUT);
        if (res != ESP_OK) return res;
    #endif
        
    res = gpio_set_direction(CONFIG_PIN_NUM_ICE40_CS, GPIO_MODE_OUTPUT);
    if (res != ESP_OK) return res;
        
    /*if (!driver_ice40_get_done()) {
        printf("ICE40: No bitstream loaded, putting FPGA in RESET state.\n");
        res = driver_ice40_disable();
        if (res != ESP_OK) return res;
    } else {
        printf("ICE40: A bitstream has already been loaded, FPGA is active.\n");
        res = driver_ice40_register_device(true);
        if (res != ESP_OK) return res;
    }
    
    return ESP_OK;*/
    
    return driver_ice40_disable(); // Always disable the FPGA on boot
}

#else
esp_err_t driver_ice40_init(void) { return ESP_OK; } //Dummy function.
#endif // CONFIG_DRIVER_ICE40_ENABLE
