#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>

#define ERC12864_WIDTH  128
#define ERC12864_HEIGHT 64
#define ERC12864_BUFFER_SIZE (ERC12864_WIDTH * ERC12864_HEIGHT) / 8

extern esp_err_t driver_erc12864_init(void);
extern esp_err_t driver_erc12864_write(const uint8_t *data);
extern esp_err_t driver_erc12864_write_part(const uint8_t *data, int16_t x0, int16_t y0, int16_t x1, int16_t y1);
