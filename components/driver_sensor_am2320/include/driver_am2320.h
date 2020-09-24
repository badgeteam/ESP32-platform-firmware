/*
 * Driver for the AM2320 I2C temperature and humidity sensor
 * Jeroen Bos (Hieronymox)
 *
 */

#ifndef DRIVER_AM2320_H
#define DRIVER_AM2320_H

#include <stdint.h>
#include <esp_err.h>

#define CACHE_TIMEOUT_MS 10 * (1000 * 1000)  // 10 seconds
#define SENSOR_NAN_VALUE 0xFFFF

#ifdef CONFIG_DRIVER_AM2320_ENABLE
#if CONFIG_I2C_MASTER_FREQ_HZ > 100000
#error \
    "I2C interface speed is set to more than 100kHz, the AM2320 sensor supports a speed of at most 100kHz."
#endif
#endif

__BEGIN_DECLS

extern esp_err_t driver_am2320_init(void);
extern float driver_am2320_get_temperature();
extern float driver_am2320_get_humidity();
extern esp_err_t driver_am2320_read_sensor(float *temperature, float *humidity);

__END_DECLS

#endif  // DRIVER_AM2320_H
