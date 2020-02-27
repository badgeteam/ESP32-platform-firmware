#ifndef DRIVER_PCA9555_H
#define DRIVER_PCA9555_H

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>

__BEGIN_DECLS

#define PCA9555_REG_INPUT_0    0
#define PCA9555_REG_INPUT_1    1
#define PCA9555_REG_OUTPUT_0   2
#define PCA9555_REG_OUTPUT_1   3
#define PCA9555_REG_POLARITY_0 4
#define PCA9555_REG_POLARITY_1 5
#define PCA9555_REG_CONFIG_0   6
#define PCA9555_REG_CONFIG_1   7

typedef void (*driver_pca9555_intr_t)(uint8_t, bool); // Interrupt handler type

/**
 * Initialize internal structures and interrupt-handling for the PCA9555.
 * @note This should be called before using any other methods.
 *   The only exception is driver_pca9555_set_interrupt_handler().
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_pca9555_init(void);

/**
 * Configure interrupt handler for touch events.
 * @param handler the handler to be called on an interrupt.
 * @note It is safe to set the interrupt handler before a call to driver_pca9555_init().
 */
extern void driver_pca9555_set_interrupt_handler(uint8_t pin, driver_pca9555_intr_t handler);

/**
 * Configure the direction of a pin
 * @param pin the pin-number (0-15)
 * @param direction 0 is input, 1 is output
 * @return 0 on success; -1 on error
 */
extern int driver_pca9555_set_gpio_direction(int pin, bool direction);

/**
 * Retrieve the current direction of a pin
 * @param pin the pin-number (0-15)
 * @return 0 when input; 1 when output; -1 on error
 */
extern int driver_pca9555_get_gpio_direction(int pin);

/**
 * Configure the polarity of a pin
 * @param pin the pin-number (0-15)
 * @param direction 0 is normal, 1 is inverted
 * @return 0 on success; -1 on error
 */
extern int driver_pca9555_set_gpio_polarity(int pin, bool polarity);

/**
 * Retrieve the current polarity of a pin
 * @param pin the pin-number (0-15)
 * @return 0 when normal; 1 when inverted; -1 on error
 */
extern int driver_pca9555_get_gpio_polarity(int pin);

/**
 * Set the state of a GPIO pin
 * @param pin the pin-number
 * @param value 0 is low; 1 is high
 * @return 0 on succes; -1 on error
 */
extern int driver_pca9555_set_gpio_value(int pin, bool value);

/**
 * Retrieve the current state of a GPIO pin
 * @param pin the pin-number
 * @return 0 when low; 1 when high; -1 on error
 */
extern int driver_pca9555_get_gpio_value(int pin);

__END_DECLS

#endif
