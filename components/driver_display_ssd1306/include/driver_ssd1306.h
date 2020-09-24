#ifndef DRIVER_SSD1306_H
#define DRIVER_SSD1306_H

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>

#define SSD1306_WIDTH  128

#ifdef CONFIG_SSD1306_12832
	#define SSD1306_HEIGHT 32
#else
	#define SSD1306_HEIGHT 64
#endif

#define SSD1306_BUFFER_SIZE (SSD1306_WIDTH * SSD1306_HEIGHT) / 8

__BEGIN_DECLS

extern esp_err_t driver_ssd1306_init(void);
extern esp_err_t driver_ssd1306_write_part(const uint8_t *buffer, int16_t x0, int16_t y0, int16_t x1, int16_t y1);
extern esp_err_t driver_ssd1306_write(const uint8_t *buffer);

__END_DECLS

#endif // DRIVER_SSD1306_H
