#ifndef PLATFORM_H
#define PLATFORM_H

#include "include/system.h"

#define INIT_DRIVER(name) { extern esp_err_t driver_##name##_init(void); if (driver_##name##_init() != ESP_OK) restart(); }
#define TEST_DRIVER(name) { extern esp_err_t driver_##name##_test(void); if (driver_##name##_test() != ESP_OK) return false; }

void platform_init( void );

#endif
