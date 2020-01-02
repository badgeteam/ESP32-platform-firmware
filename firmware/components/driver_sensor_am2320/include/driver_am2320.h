/*
 * Driver for the AM2320 I2C temperature and humidity sensor
 * Jeroen Bos (Hieronymox)
 *
 */

#ifndef DRIVER_AM2320_H
#define DRIVER_AM2320_H

#include <stdint.h>
#include <esp_err.h>

#define CACHE_TIMEOUT_MS 10 * (1000 * 1000) // 10 seconds
#define SENSOR_NAN_VALUE 0xFFFF
__BEGIN_DECLS

extern esp_err_t driver_am2320_init(void);
extern esp_err_t driver_am2320_get_temperature(float *temperature);
extern esp_err_t driver_am2320_get_humidity(float *humidity);
extern esp_err_t driver_am2320_read_sensor(float *temperature, float *humidity);

__END_DECLS

#endif  // DRIVER_AM2320_H
