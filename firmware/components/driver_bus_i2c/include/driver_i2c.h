#ifndef DRIVER_I2C_H
#define DRIVER_I2C_H

#include <stdint.h>
#include <esp_err.h>

__BEGIN_DECLS

/** initialize i2c bus
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_i2c_init(void);

extern esp_err_t driver_i2c_read_bytes(uint8_t addr, uint8_t *value, size_t value_len);

/** read register via i2c bus
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t *value, size_t value_len);

/** write to register via i2c bus
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_i2c_write_byte(uint8_t addr, uint8_t value);
extern esp_err_t driver_i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t value);
extern esp_err_t driver_i2c_write_reg_n(uint8_t addr, uint8_t reg, uint8_t *value, size_t value_len);
extern esp_err_t driver_i2c_write_reg32(uint8_t addr, uint8_t reg, uint32_t value);
extern esp_err_t driver_i2c_write_buffer(uint8_t addr, const uint8_t* buffer, uint16_t len);
extern esp_err_t driver_i2c_write_buffer_reg(uint8_t addr, uint8_t reg, const uint8_t* buffer, uint16_t len);

/** read event via i2c bus
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_i2c_read_event(uint8_t addr, uint8_t *buf);

__END_DECLS

#endif // DRIVER_I2C_H
