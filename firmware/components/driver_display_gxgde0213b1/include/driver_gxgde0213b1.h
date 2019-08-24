#ifndef DRIVER_GXGDE0213B1_H
#define DRIVER_GXGDE0213B1_H

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>

#define GXGDE0213B1_WIDTH  128
#define GXGDE0213B1_HEIGHT 250

#define GXGDE0213B1_BUFFER_SIZE GXGDE0213B1_WIDTH * GXGDE0213B1_HEIGHT / 8

__BEGIN_DECLS

extern esp_err_t driver_gxgde0213b1_init(void);
extern esp_err_t driver_gxgde0213b1_set_backlight(bool state);
extern esp_err_t driver_gxgde0213b1_set_sleep(bool state);
extern esp_err_t driver_gxgde0213b1_set_display(bool state);
extern esp_err_t driver_gxgde0213b1_set_invert(bool state);
extern esp_err_t driver_gxgde0213b1_write(const uint8_t *data);
extern esp_err_t driver_gxgde0213b1_write_partial(const uint8_t *buffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

__END_DECLS

#endif // DRIVER_GXGDE0213B1_H
