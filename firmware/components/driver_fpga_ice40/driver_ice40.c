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

esp_err_t driver_ice40_send(const uint8_t *data, int len) {
    if (len == 0) return ESP_OK;
    spi_transaction_t t = {
        .length = len * 8,  // transaction length is in bits
        .tx_buffer = data,
        .rx_buffer = NULL
    };
    return spi_device_transmit(spiDevice, &t);
}

esp_err_t driver_ice40_receive(uint8_t *data, int len) {
    if (len == 0) return ESP_OK;
    spi_transaction_t t = {
        .length = len * 8,  // transaction length is in bits
        .rxlength = len * 8,
        .rx_buffer = data,
        .tx_buffer = NULL
    };
    return spi_device_transmit(spiDevice, &t);
}

esp_err_t driver_ice40_disable() {
    esp_err_t res = ESP_FAIL;
    #ifdef CONFIG_PIN_NUM_ICE40_RESET
        res = gpio_set_level(CONFIG_PIN_NUM_ICE40_RESET, false);
        if (res != ESP_OK) printf("driver_ice40_disable: failed to set RESET state\n");
        if (res != ESP_OK) return res;
    #endif
    #ifdef CONFIG_PIN_NUM_ICE40_RESET_PCA9555
        res = driver_pca9555_set_gpio_value(CONFIG_PIN_NUM_ICE40_RESET_PCA9555, false);
        if (res != 0) printf("driver_ice40_disable: failed to set RESET state on PCA9555\n");
        if (res != 0) return ESP_FAIL;
    #endif
    if (!spiDeviceHasChipSelect) {
        res = gpio_set_level(CONFIG_PIN_NUM_ICE40_CS, true);
        if (res != ESP_OK) printf("driver_ice40_disable: failed to set cs state\n");
        if (res != ESP_OK) return res;
    }
    printf("driver_ice40_disable: done\n");
    return ESP_OK;
}

esp_err_t driver_ice40_enable() {
    esp_err_t res = ESP_FAIL;
    #ifdef CONFIG_PIN_NUM_ICE40_RESET
        res = gpio_set_level(CONFIG_PIN_NUM_ICE40_RESET, true);
    #endif
    #ifdef CONFIG_PIN_NUM_ICE40_RESET_PCA9555
        res = driver_pca9555_set_gpio_value(CONFIG_PIN_NUM_ICE40_RESET_PCA9555, true) ? ESP_FAIL : ESP_OK;
    #endif
    return res;
}

int driver_ice40_get_done(void) {
    #ifdef CONFIG_PIN_NUM_ICE40_DONE
        return !gpio_get_level(CONFIG_PIN_NUM_ICE40_DONE);
    #endif
    #ifdef CONFIG_PIN_NUM_ICE40_DONE_PCA9555
        return !driver_pca9555_get_gpio_value(CONFIG_PIN_NUM_ICE40_DONE_PCA9555);
    #endif
    return 0;
}

esp_err_t driver_ice40_register_device(bool enableChipSelect) {
    if (spiDeviceHasChipSelect && enableChipSelect && (spiDevice != NULL)) {
        // We're already in the correct mode
        printf("driver_ice40_register_device: already registered with cs\n");
        return ESP_OK;
    }
    if ((!spiDeviceHasChipSelect) && (!enableChipSelect) && (spiDevice != NULL)) {
        // We're already in the correct mode
        printf("driver_ice40_register_device: already registered without cs\n");
        return ESP_OK;
    }
    esp_err_t res = ESP_FAIL;
    if (spiDevice != NULL) {
        res = spi_bus_remove_device(spiDevice);
        if (res != ESP_OK) return res;
    }
    printf("driver_ice40_register_device: registering...\n");
    if (enableChipSelect) {
        static const spi_device_interface_config_t devcfg = {
            .clock_speed_hz = CONFIG_DRIVER_ICE40_SPI_SPEED,
            .mode           = 0,  // SPI mode 0
            .spics_io_num   = CONFIG_PIN_NUM_ICE40_CS,
            .queue_size     = 1,
            .flags          = SPI_DEVICE_HALFDUPLEX
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
        spiDeviceHasChipSelect = false;
    }
    if (res == ESP_OK) {
      printf("driver_ice40_register_device: done\n");
    } else {
      printf("driver_ice40_register_device: failed\n");;  
    }
    return res;
}

esp_err_t driver_ice40_load_bitstream(uint8_t* bitstream, uint32_t length) {
    esp_err_t res = driver_ice40_disable(); // Put ICE40 in reset state
    if (res != ESP_OK) return res;
    res = driver_ice40_register_device(false); // Disable the CS pin for normal SPI transfers
    if (res != ESP_OK) return res;
    res = gpio_set_level(CONFIG_PIN_NUM_ICE40_CS, false); // Set CS pin low
    if (res != ESP_OK) return res;
    res = driver_ice40_enable(); // Put ICE40 in SPI slave download state
    if (res != ESP_OK) return res;
    uint32_t position = 0;
    printf("Bitstream length is %u\n", length);
    while (position < length) {
        uint32_t remaining = length - position;
        uint32_t transferSize = (remaining < CONFIG_DRIVER_VSPI_MAX_TRANSFERSIZE) ? remaining : CONFIG_DRIVER_VSPI_MAX_TRANSFERSIZE;
        printf("Sending %u bytes @ %u\n", transferSize, position);
        driver_ice40_send(bitstream + position, transferSize);
        position += transferSize;
    }
    for (uint8_t i = 0; i < 10; i++) {
        printf("Sending 10 dummy bytes\n");
        const uint8_t dummy[10] = {0};
        driver_ice40_send(dummy, sizeof(dummy));
    }
    res = gpio_set_level(CONFIG_PIN_NUM_ICE40_CS, false); // Set CS pin high
    if (res != ESP_OK) return res;
    if (driver_ice40_get_done()) {
        printf("ICE40 signals DONE\n");
        res = driver_ice40_register_device(true); // Enable the CS pin for normal SPI transfers
        if (res != ESP_OK) return res;
        res = ESP_OK;
    } else {
        printf("ICE40 signals NOT DONE\n");
        res = ESP_FAIL;
    }
    return res;
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
        
    if (!driver_ice40_get_done()) {
        printf("ICE40: No bitstream loaded, putting FPGA in RESET state.\n");
        res = driver_ice40_register_device(false);
        if (res != ESP_OK) return res;
        res = driver_ice40_disable();
        if (res != ESP_OK) return res;
    } else {
        printf("ICE40: A bitstream has already been loaded, FPGA is active.\n");
        res = driver_ice40_register_device(true);
        if (res != ESP_OK) return res;
    }
    
    return ESP_OK;
}

#else
esp_err_t driver_ice40_init(void) { return ESP_OK; } //Dummy function.
#endif // CONFIG_DRIVER_ICE40_ENABLE
