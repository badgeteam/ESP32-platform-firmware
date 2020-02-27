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
 * @note It is safe to set the interrupt handler before a call to driver_pca95xx_init().
 */
extern void driver_pca9555_set_interrupt_handler(uint8_t pin, driver_pca9555_intr_t handler);

__END_DECLS

#endif
