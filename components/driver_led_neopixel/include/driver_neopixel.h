#ifndef DRIVER_NEOPIXEL_H
#define DRIVER_NEOPIXEL_H

#include <stdint.h>
#include <esp_err.h>

__BEGIN_DECLS

/**
 * Initialize the leds driver. (configure SPI bus and GPIO pins)
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_neopixel_init(void);

/**
 * Enable power to the leds bar.
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_neopixel_enable(void);

/**
 * Disable power to the leds bar.
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_neopixel_disable(void);

/**
 * Send color-data to the leds bus.
 * @param data the data-bytes to send on the bus.
 * @param len the data-length.
 * @note The first 6 leds on the bus are probably SK6812RGBW leds.
 *   SK6812RGBW expects 4 bytes per led: green, red, blue and white.
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_neopixel_send_data(uint8_t *data, int len);

__END_DECLS

#endif // DRIVER_NEOPIXEL_H
