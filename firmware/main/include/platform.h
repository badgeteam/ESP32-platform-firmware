#ifndef PLATFORM_H
#define PLATFORM_H

#include "include/system.h"

#define INIT_DRIVER(x) { extern esp_err_t driver_##x##_init(void); if (driver_##x##_init() != ESP_OK) restart(); }

void platform_init( void );

#endif
