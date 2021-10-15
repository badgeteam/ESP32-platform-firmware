#pragma once

#include <stdint.h>
#include <esp_err.h>

/**
 * Initialize the Neopixel LED driver. (configure SPI bus and GPIO pins)
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_neopixel_init(void);

/**
 * Enable power to the LEDs
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_neopixel_enable(void);

/**
 * Disable power to the LEDs
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_neopixel_disable(void);

/**
 * Send color-data to the LEDs
 * @param data the data-bytes to send on the bus.
 * @param len the data-length.
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_neopixel_send_data(uint8_t *data, int len);
