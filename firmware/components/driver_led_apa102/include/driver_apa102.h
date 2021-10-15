#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>

/**
 * Initialize the APA102 LED driver. (configure SPI bus)
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_apa102_init(void);

/**
 * Send color-data to the leds
 * @param data the data-bytes to send on the bus.
 * @param len the data-length.
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_apa102_send_data(uint8_t *data, int len);

/**
 * Set the global brightness setting
 * @param value brightness
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_apa102_set_brightness(uint8_t value);
