#ifndef PLATFORM_H
#define PLATFORM_H

#include "include/system.h"

#include "esp_log.h"

#define INIT_DRIVER(name,message) { extern esp_err_t driver_##name##_init(void); if (driver_##name##_init() != ESP_OK) fatal_error(message); }
void platform_init( void );

#endif
