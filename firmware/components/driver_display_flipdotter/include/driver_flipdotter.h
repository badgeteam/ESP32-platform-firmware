#ifndef DRIVER_FLIPDOTTER_H
#define DRIVER_FLIPDOTTER_H

#include <stdint.h>
#include <esp_err.h>

__BEGIN_DECLS

#define FLIPDOTTER_BUFFER_SIZE (CONFIG_FLIPDOTTER_COLS*CONFIG_FLIPDOTTER_ROWS*CONFIG_FLIPDOTTER_MODULES) / 8
#define FLIPDOTTER_WIDTH CONFIG_FLIPDOTTER_COLS*CONFIG_FLIPDOTTER_MODULES
#define FLIPDOTTER_HEIGHT CONFIG_FLIPDOTTER_ROWS

extern esp_err_t driver_flipdotter_init(void);
extern esp_err_t driver_flipdotter_write(const uint8_t *buffer);
extern esp_err_t driver_flipdotter_set_backlight(uint8_t brightness);

__END_DECLS

#endif
