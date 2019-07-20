#ifndef _MPU_DRIVER_H_
#define _MPU_DRIVER_H_

#include "../../driver_bus_i2c/include/driver_i2c.h"
#include <stdbool.h>

int init_mpu();

void getAccel(int *x, int *y, int *z);


#endif