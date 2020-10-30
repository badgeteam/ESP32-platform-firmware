#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>

extern esp_err_t driver_mch2021_stm32_init();
esp_err_t driver_mch2021_stm32_transaction(const uint8_t* out, uint8_t* in);
