#ifndef DRIVER_I2C_DISPLAY
#define DRIVER_I2C_DISPLAY

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>

#define I2C_DISPLAY_WIDTH  CONFIG_DISPLAY_I2C_WIDTH
#define I2C_DISPLAY_HEIGHT CONFIG_DISPLAY_I2C_HEIGHT
#define I2C_DISPLAY_BUFFER_SIZE ((CONFIG_DISPLAY_I2C_WIDTH * CONFIG_DISPLAY_I2C_HEIGHT) * 3)

__BEGIN_DECLS

extern esp_err_t driver_i2c_display_init(void);
extern esp_err_t driver_i2c_display_write(const uint8_t *buffer);

__END_DECLS

#endif // DRIVER_I2C_DISPLAY
