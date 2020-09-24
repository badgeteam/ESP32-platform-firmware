#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>

extern esp_err_t driver_ice40_init();
extern esp_err_t driver_ice40_get_done();
extern esp_err_t driver_ice40_load_bitstream();
