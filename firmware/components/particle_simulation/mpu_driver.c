#include "include/mpu_driver.h"

const uint8_t addr = 0x68;


esp_err_t init_mpu() {
    driver_i2c_write_reg(addr, 0x6B, 0b10000001); //Set CLKSEL to X gyro and keep sleep
    driver_i2c_write_reg(addr, 0x1c, 0b00000000); //Set accel range to 2g
    driver_i2c_write_reg(addr, 0x1A, 6);    //Enable the 5KHz lpf
    driver_i2c_write_reg(addr, 0x19, 0);    //Set divider to 0
    return driver_i2c_write_reg(addr, 0x6B, 0b00000001);   //Power up chip
}

void getAccel(int *x, int *y, int *z) {
    int16_t data[3];
    driver_i2c_read_reg(addr, 0x3b, (uint8_t *) data, 6);
    *x = data[0];
    *y = data[1];
    *z = data[2];
}