/*
 * Driver for the AM2320 I2C temperature and humidity sensor
 * Jeroen Bos (Hieronymox)
 *
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <sdkconfig.h>
#include <driver/gpio.h>

#include <include/driver_i2c.h>
#include "include/driver_am2320.h"

#ifdef CONFIG_DRIVER_AM2320_ENABLE

static const char *TAG              = "driver_am2320";
static bool driver_am2320_init_done = false;

static float cached_temperature = 0;
static float cached_humidity    = 0;
static uint64_t cache_time      = 0;

esp_err_t driver_am2320_init(void) {
  if (driver_am2320_init_done)
    return ESP_OK;

  ESP_LOGD(TAG, "init called");

  driver_am2320_init_done = true;

  ESP_LOGD(TAG, "init done");
  return ESP_OK;
}

static uint16_t calculate_crc(uint8_t *buffer, uint8_t num_bytes) {
  uint16_t crc = 0xFFFF;
  while (num_bytes--) {
    crc ^= *buffer++;
    for (uint8_t i = 0; i < 8; i++) {
      if (crc & 0x01) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

// Checks if the values are to old and updates them
static void update_cache() {
  if (cache_time != 0 && (cache_time + CACHE_TIMEOUT_MS > esp_timer_get_time())) {
    // data still up to date
    return;
  }
  driver_am2320_read_sensor(&cached_temperature, &cached_humidity);
  cache_time = esp_timer_get_time();
}

// Uses cached value
float driver_am2320_get_temperature() {
  update_cache();
  return cached_temperature;
}

// Uses cached value
float driver_am2320_get_humidity() {
  update_cache();
  return cached_humidity;
}

// Does actual sensor reading and provide temperature and humidity, this call takes 2 seconds and
// running it to often can cause internal heating in the sensor. Please use the get_temperature() or
// get_humidity() unless you always want the most recent data.
esp_err_t driver_am2320_read_sensor(float *temperature, float *humidity) {
  driver_i2c_write_byte(CONFIG_DRIVER_AM2320_I2C_ADDRESS, 0x00);  // Wakeup sensor
  vTaskDelay(2 / portTICK_RATE_MS);

  uint8_t read_config[3];
  read_config[0] = 0x03;  // Function
  read_config[1] = 0x00;  // Full register
  read_config[2] = 4;     // Number of registers to read
  driver_i2c_write_buffer(CONFIG_DRIVER_AM2320_I2C_ADDRESS, read_config, 3);
  vTaskDelay(2000 / portTICK_RATE_MS);  // Delay to allow sensor to measure (takes almost 2 seconds)

  // 0: 0x03 Modbus byte
  // 1: 0x04 No. bytes data
  // 2+3: Humidity
  // 4+5: Temperature
  // 6+7: CRC
  uint8_t result[8] = {0};
  driver_i2c_read_bytes(CONFIG_DRIVER_AM2320_I2C_ADDRESS, result, 8);

  // Check if we get expected data, if not return NAN
  if (result[0] != 0x03 || (result[2] == 0xFF && result[3] == 0xFF)) {
    ESP_LOGD(TAG, "Data does not match expected format");
    *humidity    = SENSOR_NAN_VALUE;
    *temperature = SENSOR_NAN_VALUE;
    return ESP_ERR_INVALID_RESPONSE;
  }

  // Validate the crc, if failed return NAN
  uint16_t received_crc   = result[6] + result[7] * 256;  // CRC uses low/high byte
  uint16_t calculated_crc = calculate_crc(result, 6);
  if (received_crc != calculated_crc) {
    ESP_LOGD(TAG, "CRC check on data failed");
    *humidity    = SENSOR_NAN_VALUE;
    *temperature = SENSOR_NAN_VALUE;
    return ESP_ERR_INVALID_CRC;
  }

  uint16_t humidity_raw    = result[2] * 256 + result[3];  // Sensor data uses high/low byte
  uint16_t temperature_raw = result[4] * 256 + result[5];  // Sensor data uses high/low byte

  if (temperature_raw & 0x8000)  // check if minus bit is set
    temperature_raw = -(temperature_raw & 0x7FFF);

  *humidity    = (float)humidity_raw / 10;
  *temperature = (float)temperature_raw / 10;

  return ESP_OK;
}

#else
esp_err_t driver_am2320_init(void) {
  return ESP_OK;
}
#endif
