#ifndef DRIVER_OPENAARS_H
#define DRIVER_OPENAARS_H

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>

// Interrupt number identifier
#define INT_OPENAARS_ADV_NR  0
#define INT_OPENAARS_FPGA_NR 1
#define INT_OPENAARS_LAST_NR 2

__BEGIN_DECLS

// Interrupt routines
typedef void (*driver_openaars_intr_t)(void*, bool); // Interrupt handler type

extern void driver_openaars_set_interrupt_handler(uint8_t pin, driver_openaars_intr_t handler, void *arg);

// SPI routines
extern esp_err_t driver_openaars_init(void);
extern esp_err_t driver_openaars_send(const uint8_t *data, int len, const uint8_t dc_level);
extern esp_err_t driver_openaars_receive(uint8_t *data, int len, const uint8_t dc_level);
extern esp_err_t driver_openaars_reset(void);
extern esp_err_t driver_openaars_send_command(uint8_t cmd);
__END_DECLS

#endif // DRIVER_OPENAARS_H
