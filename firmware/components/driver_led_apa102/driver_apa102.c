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

#include "include/driver_apa102.h"

#ifdef CONFIG_DRIVER_APA102_ENABLE

static spi_device_handle_t spiDevice = NULL;
static uint8_t brightness = 0x1F;

esp_err_t driver_apa102_set_brightness(uint8_t value) {
    brightness = value;
    return ESP_OK;
}

esp_err_t driver_apa102_send(const uint8_t *data, int length) {
    esp_err_t result = ESP_OK;
    if (length > 0) {
        spi_transaction_t t = {
            .length = length * 8,  // transaction length is in bits
            .tx_buffer = data,
            .rx_buffer = NULL
        };
        result = spi_device_transmit(spiDevice, &t);
    }
    return result;
}

esp_err_t driver_apa102_send_data(uint8_t *data, int dataLength) {
    esp_err_t result = ESP_FAIL;

    if (!(dataLength % 3)) { // Data must be three bytes per LED (Red, Green, Blue)
        const uint16_t preambleLength = 4; // 4x zero
        uint16_t numLeds = dataLength / 3;
        uint16_t postambleLength = (numLeds / 16) + 1; // At least (numLeds / 2) + 1 bits, this value is the size in bytes
        uint16_t packetLength = preambleLength + (numLeds * 4) + postambleLength;
        uint8_t* packet = (uint8_t*) malloc(packetLength);
        if (packet != NULL) {
            memset(packet, 0x00, preambleLength); // Preamble is zero
            for (uint16_t led = 0; led < numLeds; led++) {
                packet[preambleLength + (led * 4) + 0] = 0xE0 + (brightness & 0x1F);
                packet[preambleLength + (led * 4) + 1] = data[(led * 3) + 2];
                packet[preambleLength + (led * 4) + 2] = data[(led * 3) + 1];
                packet[preambleLength + (led * 4) + 3] = data[(led * 3) + 0];
            }
            memset(packet + preambleLength + (numLeds * 4), 0xFF, postambleLength); // Postamble is all bits on (0xFF)
            result = driver_apa102_send(packet, packetLength);
            free(packet);
        } else {
            result = ESP_ERR_NO_MEM;
        }
    }

    return result;
}

esp_err_t driver_apa102_init(void) {	
    esp_err_t result = ESP_OK;

    static const spi_device_interface_config_t devcfg = {
        .clock_speed_hz = CONFIG_DRIVER_APA102_SPI_SPEED,
        .mode           = 0,
        .spics_io_num   = -1,
        .queue_size     = 1,
        .flags          = SPI_DEVICE_HALFDUPLEX
    };
    result = spi_bus_add_device(VSPI_HOST, &devcfg, &spiDevice);

    return result;
}

#else
esp_err_t driver_apa102_init(void) { return ESP_OK; } //Dummy function.
#endif
