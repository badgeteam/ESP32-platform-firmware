#include "include/mpu_driver.h"

const uint8_t addr = 0x68;


esp_err_t init_mpu() {
    if(driver_i2c_write_reg(addr, 0x19, 0x00) != ESP_OK) return ESP_FAIL;
    if(driver_i2c_write_reg(addr, 0x1a, 0x00) != ESP_OK) return ESP_FAIL;
    if(driver_i2c_write_reg(addr, 0x1b, 0x08) != ESP_OK) return ESP_FAIL;
    if(driver_i2c_write_reg(addr, 0x1c, 0x00) != ESP_OK) return ESP_FAIL;
    return driver_i2c_write_reg(addr, 0x6b, 0x01);
}

void getAccel(int *x, int *y, int *z) {
    uint8_t data[6];
    uint8_t p = driver_i2c_read_reg(addr, 0x3b, (uint8_t *) data, 6);
    *x = ((uint16_t) data[0]) << 8 | ((uint16_t) data[1]);
    *y = ((uint16_t) data[2]) << 8 | ((uint16_t) data[3]);
    *z = ((uint16_t) data[4]) << 8 | ((uint16_t) data[5]);
}
