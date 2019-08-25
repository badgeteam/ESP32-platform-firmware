#include <sdkconfig.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <esp_err.h>

#include <driver_i2c.h>

#include "include/driver_mpu6050.h"

#ifdef CONFIG_DRIVER_MPU6050_ENABLE

#define TAG "mpu6050"

esp_err_t driver_mpu6050_configure_dlpf(uint8_t dlpf) {
	esp_err_t res = driver_mpu6050_init();
	if (res != ESP_OK) return res;
	res = driver_i2c_write_reg(CONFIG_I2C_ADDR_MPU6050, MPU6050_CONFIG, (dlpf&0x07)); //We ignore EXT_SYNC here, it is set to disabled
	return res;
}

esp_err_t driver_mpu6050_configure_gyro_range(uint8_t fs_sel) {
	esp_err_t res = driver_mpu6050_init();
	if (res != ESP_OK) return res;
	res = driver_i2c_write_reg(CONFIG_I2C_ADDR_MPU6050, MPU6050_GYRO_CONFIG, (fs_sel&0x03)<<3); //Set full scale range, ignore self-test features
	return res;
}

esp_err_t driver_mpu6050_configure_accel_range(uint8_t afs_sel) {
	esp_err_t res = driver_mpu6050_init();
	if (res != ESP_OK) return res;
	res = driver_i2c_write_reg(CONFIG_I2C_ADDR_MPU6050, MPU6050_ACCEL_CONFIG, (afs_sel&0x03)<<3); //Set full scale range, ignore self-test features
	return res;
}

esp_err_t driver_mpu6050_interrupt_configure(bool int_level, bool int_open, bool latch_int_en, bool int_rd_clear, bool fsync_int_level, bool fsync_int_en, bool i2c_bypass_en) {
	esp_err_t res = driver_mpu6050_init();
	if (res != ESP_OK) return res;
	
	uint8_t value = 0;
	if (int_level)       value += 1 << 1;
	if (int_open)        value += 1 << 2;
	if (latch_int_en)    value += 1 << 3;
	if (int_rd_clear)    value += 1 << 4;
	if (fsync_int_level) value += 1 << 5;
	if (fsync_int_en)    value += 1 << 6;
	if (i2c_bypass_en)   value += 1 << 7;
	
	res = driver_i2c_write_reg(CONFIG_I2C_ADDR_MPU6050, MPU6050_INT_PIN_CFG, value);
	return res;
}

esp_err_t driver_mpu6050_interrupt_enable(bool fifo_oflow_en, bool i2c_mst_int_en, bool data_rdy_en) {
	esp_err_t res = driver_mpu6050_init();
	if (res != ESP_OK) return res;
	
	uint8_t value = 0;
	if (data_rdy_en)     value += 1 << 0;
	if (i2c_mst_int_en)  value += 1 << 3;
	if (data_rdy_en)     value += 1 << 4;
	
	res = driver_i2c_write_reg(CONFIG_I2C_ADDR_MPU6050, MPU6050_INT_ENABLE, value);
	return res;
}

esp_err_t driver_mpu6050_read_interrupt_status(bool *fifo_oflow_int, bool *i2c_mst_int, bool* data_rdy_int) {
	esp_err_t res = driver_mpu6050_init();
	if (res != ESP_OK) return res;
	uint8_t data[1];
	res = driver_i2c_read_reg(CONFIG_I2C_ADDR_MPU6050, MPU6050_INT_STATUS, (uint8_t *) data, 1);
	if (res != ESP_OK) return res;
	*data_rdy_int   = (data[0]&0x01)>>0;
	*i2c_mst_int    = (data[0]&0x08)>>3;
	*fifo_oflow_int = (data[0]&0x10)>>4;
	return ESP_OK;
}

esp_err_t driver_mpu6050_read_accel(int16_t *x, int16_t *y, int16_t *z) {
	esp_err_t res = driver_mpu6050_init();
	if (res != ESP_OK) return res;
	uint8_t data[6];
	res = driver_i2c_read_reg(CONFIG_I2C_ADDR_MPU6050, MPU6050_ACCEL_XOUT_H, (uint8_t *) data, 6);
	if (res != ESP_OK) return res;
	*x = (data[0] << 8) | data[1];
	*y = (data[2] << 8) | data[3];
	*z = (data[4] << 8) | data[5];
	return ESP_OK;
}

esp_err_t driver_mpu6050_read_gyro(int16_t *x, int16_t *y, int16_t *z) {
	esp_err_t res = driver_mpu6050_init();
	if (res != ESP_OK) return res;
	uint8_t data[6];
	res = driver_i2c_read_reg(CONFIG_I2C_ADDR_MPU6050, MPU6050_GYRO_XOUT_H, (uint8_t *) data, 6);
	if (res != ESP_OK) return res;
	*x = (data[0] << 8) | data[1];
	*y = (data[2] << 8) | data[3];
	*z = (data[4] << 8) | data[5];
	return ESP_OK;
}

esp_err_t driver_mpu6050_read_temp(float *t) {
	esp_err_t res = driver_mpu6050_init();
	if (res != ESP_OK) return res;
	uint8_t data[2];
	res = driver_i2c_read_reg(CONFIG_I2C_ADDR_MPU6050, MPU6050_TEMP_OUT_H, (uint8_t *) data, 2);
	if (res != ESP_OK) return res;
	
	int16_t value = (data[0] << 8) | data[1];
	*t = (value / 340) + 36.53;
	return ESP_OK;
}

static inline esp_err_t _init(void)
{
	if ( driver_i2c_write_reg(CONFIG_I2C_ADDR_MPU6050, MPU6050_SMPRT_DIV,    0x00) != ESP_OK ) return ESP_FAIL;
	if ( driver_i2c_write_reg(CONFIG_I2C_ADDR_MPU6050, MPU6050_CONFIG,       0x00) != ESP_OK ) return ESP_FAIL;
	if ( driver_i2c_write_reg(CONFIG_I2C_ADDR_MPU6050, MPU6050_GYRO_CONFIG,  0x08) != ESP_OK ) return ESP_FAIL;
	if ( driver_i2c_write_reg(CONFIG_I2C_ADDR_MPU6050, MPU6050_ACCEL_CONFIG, 0x00) != ESP_OK ) return ESP_FAIL;
	if ( driver_i2c_write_reg(CONFIG_I2C_ADDR_MPU6050, MPU6050_PWR_MGMT_1,   0x01) != ESP_OK ) return ESP_FAIL;
	return ESP_OK;
}

esp_err_t driver_mpu6050_init(void)
{
	static bool driver_mpu6050_init_done = false;
	if (driver_mpu6050_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	esp_err_t res = _init(); //Call the real init function
	
	if (res != ESP_OK) {
		#ifdef CONFIG_DRIVER_MPU6050_IGNORE_FAILED
			ESP_LOGE(TAG, "(MPU6050 initialisation failed)");
			return ESP_OK;
		#else
			return res;
		#endif
	}
	
	driver_mpu6050_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

#else // CONFIG_DRIVER_MPU6050_ENABLE
esp_err_t driver_mpu6050_init(void) { return ESP_OK; } // Dummy function, leave empty!
#endif
