#include <sdkconfig.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <soc/gpio_reg.h>
#include <soc/gpio_sig_map.h>
#include <soc/gpio_struct.h>
#include <soc/spi_reg.h>
#include <esp_err.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>

#include "include/driver_ili9341.h"

#ifdef CONFIG_DRIVER_ILI9341_ENABLE

#define ILI9341_MAX_LINES 8

static const char *TAG = "ili9341";

uint8_t *internalBuffer; //Internal transfer buffer for doing partial updates

static spi_device_handle_t spi_bus = NULL;

static void driver_ili9341_spi_pre_transfer_callback(spi_transaction_t *t)
{
	uint8_t dc_level = *((uint8_t *) t->user);
	gpio_set_level(CONFIG_PIN_NUM_ILI9341_DCX, (int) dc_level);
}

const uint8_t ili9341_init_data[] = {
	0xEF,              3, 0x03,0x80,0x02,
	0xCF,              3, 0x00,0XC1,0X30,
	0xED,              4, 0x64,0x03,0X12,0X81,
	0xE8,              3, 0x85,0x00,0x78,
	0xCB,              5, 0x39,0x2C,0x00,0x34,0x02,
	0xF7,              1, 0x20,
	0xEA,              2, 0x00,0x00,
	ILI9341_LCMCTRL,   1, 0x23,
	0xC1,              1, 0x10,
	0xC5,              2, 0x3e,0x28,
	0xC7,              1, 0x86,
	ILI9341_MADCTL,    1, 0x48,
	ILI9341_COLMOD,    1, 0x55,
	0xB1,              2, 0x00,0x18,
	0xB6,              3, 0x08,0x82,0x27,
	0xF2,              1, 0x00,
	ILI9341_GAMSET,    1, 0x01,
	ILI9341_PVGAMCTRL, 15, 0x0F,0x31,0x2B,0x0C,0x0E,0x08,0x4E,0xF1,0x37,0x07,0x10,0x03,0x0E,0x09,0x00,
	ILI9341_NVGAMCTRL, 15, 0x00,0x0E,0x14,0x03,0x11,0x07,0x31,0xC1,0x48,0x08,0x0F,0x0C,0x31,0x36,0x0F,
	0x00
};

esp_err_t driver_ili9341_send(const uint8_t *data, int len, const uint8_t dc_level)
{
	if (len == 0) return ESP_OK;
	spi_transaction_t t = {
		.length = len * 8,  // transaction length is in bits
		.tx_buffer = data,
		.user = (void *) &dc_level,
	};
	return spi_device_transmit(spi_bus, &t);
}

esp_err_t driver_ili9341_receive(uint8_t *data, int len, const uint8_t dc_level)
{
	if (len == 0) return ESP_OK;
	spi_transaction_t t = {
		.length = len * 8,  // transaction length is in bits
		.rxlength = len * 8,
		.rx_buffer = data,
		.user = (void *) &dc_level,
	};
	return spi_device_transmit(spi_bus, &t);
}

esp_err_t driver_ili9341_write_initData(const uint8_t * data)
{
	uint8_t cmd, len;
	while(true) {
		cmd = *data++;
		if(!cmd) return ESP_OK; //END
		len = *data++;
		driver_ili9341_send(&cmd, 1, false);
		driver_ili9341_send(data, len, true);
		data+=len;
	}
	return ESP_OK;
}

esp_err_t driver_ili9341_send_command(uint8_t cmd)
{
	return driver_ili9341_send(&cmd, 1, false);
}

esp_err_t driver_ili9341_send_u32(uint32_t data)
{
	uint8_t buffer[4];
	buffer[0] = (data>>24)&0xFF;
	buffer[1] = (data>>16)&0xFF;
	buffer[2] = (data>> 8)&0xFF;
	buffer[3] = data      &0xFF;
	return driver_ili9341_send(buffer, 4, true);
}

esp_err_t driver_ili9341_set_addr_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	uint32_t xa = ((uint32_t)x << 16) | (x+w-1);
	uint32_t ya = ((uint32_t)y << 16) | (y+h-1);
	esp_err_t res;
	res = driver_ili9341_send_command(ILI9341_CASET);
	if (res != ESP_OK) return res;
	res = driver_ili9341_send_u32(xa);
	if (res != ESP_OK) return res;
	res = driver_ili9341_send_command(ILI9341_RASET);
	if (res != ESP_OK) return res;
	res = driver_ili9341_send_u32(ya);
	if (res != ESP_OK) return res;
	res = driver_ili9341_send_command(ILI9341_RAMWR);
	return res;
}

#define MADCTL_MY  0x80  ///< Bottom to topp
#define MADCTL_MX  0x40  ///< Right to left
#define MADCTL_MV  0x20  ///< Reverse Mode
#define MADCTL_ML  0x10  ///< LCD refresh Bottom to top
#define MADCTL_RGB 0x00  ///< Red-Green-Blue pixel order
#define MADCTL_BGR 0x08  ///< Blue-Green-Red pixel order
#define MADCTL_MH 0x04 ///< LCD refresh right to left

esp_err_t driver_ili9341_set_cfg(uint8_t rotation, bool colorMode)
{
	rotation = rotation & 0x03;
	uint8_t m = 0;

	switch (rotation) {
	        case 0:
	            m |= MADCTL_MX;
	            break;
	        case 1:
	            m |= MADCTL_MV;
	            break;
	        case 2:
	            m |= MADCTL_MY;
	            break;
	        case 3:
	            m |= (MADCTL_MX | MADCTL_MY | MADCTL_MV);
	            break;
	}

	if (colorMode) {
		m |= MADCTL_BGR;
	} else {
		m |= MADCTL_RGB;
	}

	uint8_t commands[2] = {ILI9341_MADCTL, m};
	esp_err_t res = driver_ili9341_send(commands, 1, false);
	if (res != ESP_OK) return res;
	res = driver_ili9341_send(commands+1, 1, true);
	return res;
}

/*esp_err_t driver_ili9341_read_id(uint32_t* result)
{
	uint8_t commands[2] = {0xD9, 0x04};
	driver_ili9341_send(commands, 1, false);
	driver_ili9341_send(commands+1, 1, true);
	uint8_t buffer[3];
	driver_ili9341_receive(buffer, 3, true);
	*result = (buffer[0]<<16)+(buffer[1]<<8)+buffer[2];
	return ESP_OK;
}*/

esp_err_t driver_ili9341_reset(void) {
	#if CONFIG_PIN_NUM_ILI9341_RESET >= 0
	esp_err_t res = gpio_set_level(CONFIG_PIN_NUM_ILI9341_RESET, false);
	if (res != ESP_OK) return res;
	ets_delay_us(200000);
	res = gpio_set_level(CONFIG_PIN_NUM_ILI9341_RESET, true);
	if (res != ESP_OK) return res;
	#endif
	ets_delay_us(200000);
	return ESP_OK;
}

esp_err_t driver_ili9341_set_backlight(bool state)
{
	#if CONFIG_PIN_NUM_ILI9341_BACKLIGHT >= 0
		return gpio_set_level(CONFIG_PIN_NUM_ILI9341_BACKLIGHT, !state);
	#else
		return ESP_OK;
	#endif
}

esp_err_t driver_ili9341_set_sleep(bool state)
{
	esp_err_t res;
	if (state) {
		res = driver_ili9341_send_command(ILI9341_SLPIN);
		if (res != ESP_OK) return res;
	} else {
		res = driver_ili9341_send_command(ILI9341_SLPOUT);
		if (res != ESP_OK) return res;
	}
	vTaskDelay(200 / portTICK_PERIOD_MS);
	return res;
}

esp_err_t driver_ili9341_set_display(bool state)
{
	esp_err_t res;
	if (state) {
		res = driver_ili9341_send_command(ILI9341_DISPON);
		if (res != ESP_OK) return res;
	} else {
		res = driver_ili9341_send_command(ILI9341_DISPOFF);
		if (res != ESP_OK) return res;
	}
	vTaskDelay(200 / portTICK_PERIOD_MS);
	return res;
}

esp_err_t driver_ili9341_set_invert(bool state)
{
	if (state) {
		return driver_ili9341_send_command(ILI9341_INVON);
	} else {
		return driver_ili9341_send_command(ILI9341_INVOFF);
	}
}

esp_err_t driver_ili9341_init(void)
{
	static bool driver_ili9341_init_done = false;
	if (driver_ili9341_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	esp_err_t res;
	
	//Initialize backlight GPIO pin
	#if CONFIG_PIN_NUM_ILI9341_BACKLIGHT>=0
		res = gpio_set_direction(CONFIG_PIN_NUM_ILI9341_BACKLIGHT, GPIO_MODE_OUTPUT);
		if (res != ESP_OK) return res;
	#endif

	//Turn off backlight
	res = driver_ili9341_set_backlight(false);
	if (res != ESP_OK) return res;

	//Allocate partial update buffer
	internalBuffer = heap_caps_malloc(CONFIG_DRIVER_VSPI_MAX_TRANSFERSIZE, MALLOC_CAP_8BIT);
	if (!internalBuffer) return ESP_FAIL;
	
	//Initialize reset GPIO pin
	#if CONFIG_PIN_NUM_ILI9341_RESET >= 0
	res = gpio_set_direction(CONFIG_PIN_NUM_ILI9341_RESET, GPIO_MODE_OUTPUT);
	if (res != ESP_OK) return res;
	#endif

	//Initialize data/clock select GPIO pin
	res = gpio_set_direction(CONFIG_PIN_NUM_ILI9341_DCX, GPIO_MODE_OUTPUT);
	if (res != ESP_OK) return res;
	
	static const spi_device_interface_config_t devcfg = {
		.clock_speed_hz = 40 * 1000 * 1000,
		.mode           = 0,  // SPI mode 0
		.spics_io_num   = CONFIG_PIN_NUM_ILI9341_CS,
		.queue_size     = 1,
		.flags          = (SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE),//SPI_DEVICE_HALFDUPLEX,
		.pre_cb         = driver_ili9341_spi_pre_transfer_callback, // Specify pre-transfer callback to handle D/C line
	};
	res = spi_bus_add_device(VSPI_HOST, &devcfg, &spi_bus);
	if (res != ESP_OK) return res;

	//Reset the LCD display
	res = driver_ili9341_reset();
	if (res != ESP_OK) return res;

	//Send the initialization data to the LCD display
	res = driver_ili9341_write_initData(ili9341_init_data);
	if (res != ESP_OK) return res;

	//Disable sleep mode
	res = driver_ili9341_set_sleep(false);
	if (res != ESP_OK) return res;

	//Turn on the LCD
	res = driver_ili9341_send_command(ILI9341_DISPON);
	if (res != ESP_OK) return res;
	
	//Configure orientation
	#ifdef CONFIG_ILI9341_COLOR_SWAP
	res = driver_ili9341_set_cfg(CONFIG_ILI9341_ORIENTATION, true);
	#else
	res = driver_ili9341_set_cfg(CONFIG_ILI9341_ORIENTATION, false);
	#endif
	if (res != ESP_OK) return res;
	
	//Turn on backlight
	res = driver_ili9341_set_backlight(true);
	if (res != ESP_OK) return res;

	driver_ili9341_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

esp_err_t driver_ili9341_write(const uint8_t *buffer)
{
	return driver_ili9341_write_partial(buffer, 0, 0, ILI9341_WIDTH, ILI9341_HEIGHT);
}

esp_err_t driver_ili9341_write_partial_direct(const uint8_t *buffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{ //Without conversion
	if (x0 > x1) return ESP_FAIL;
	if (y0 > y1) return ESP_FAIL;
	uint16_t w = x1-x0;
	uint16_t h = y1-y0;
	esp_err_t res = driver_ili9341_set_addr_window(x0, y0, w, h);
	if (res != ESP_OK) return res;
	res = driver_ili9341_send(buffer, w*h*2, true);
	return res;
}

esp_err_t driver_ili9341_write_partial(const uint8_t *frameBuffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{ //With conversion from framebuffer
	esp_err_t res = ESP_OK;
	if (x0 > x1) {
		printf("X0 %u > X1 %u\n", x0, x1);
		return ESP_FAIL;
	}
	if (y0 > y1) {
		printf("Y0 %u > Y1 %u\n", y0, y1);
		return ESP_FAIL;
	}
	if (x1 >= ILI9341_WIDTH)  x1 = ILI9341_WIDTH-1;
	if (y1 >= ILI9341_HEIGHT) y1 = ILI9341_HEIGHT-1;
	
	uint16_t w = x1-x0+1;
	uint16_t h = y1-y0+1;

#if CONFIG_DRIVER_ILI9341_8C
	while (w > 0) {
		uint16_t transactionWidth = w;
		if (transactionWidth*2 > CONFIG_DRIVER_VSPI_MAX_TRANSFERSIZE) {
			transactionWidth = CONFIG_DRIVER_VSPI_MAX_TRANSFERSIZE/2;
		}
		res = driver_ili9341_set_addr_window(x0, y0, transactionWidth, h);
		if (res != ESP_OK) return res;
		for (uint16_t currentLine = 0; currentLine < h; currentLine++) {
			for (uint16_t i = 0; i<transactionWidth; i++) {
				uint8_t color8 = frameBuffer[x0+i+(y0+currentLine)*ILI9341_WIDTH];
				uint8_t r = color8 & 0x07;
				uint8_t g = (color8>>3) & 0x07;
				uint8_t b = color8 >> 6;
				internalBuffer[i*2+0] = g | (r << 5);
				internalBuffer[i*2+1] = (b << 3);
			}
			res = driver_ili9341_send(internalBuffer, transactionWidth*2, true);
			if (res != ESP_OK) return res;
		}
		w -= transactionWidth;
		x0 += transactionWidth;
	}
#else
	//Old code
	/*while (h > 0) {
		uint16_t lines = h;
		if (lines > 1) lines = 1;
		esp_err_t res = driver_ili9341_set_addr_window(0, y0+ILI9341_OFFSET_Y, ILI9341_WIDTH, lines);
		if (res != ESP_OK) break;
		res = driver_ili9341_send(frameBuffer+(y0*ILI9341_WIDTH)*2, ILI9341_WIDTH*lines*2, true);
		if (res != ESP_OK) break;
		y0 += lines;
		h -= lines;
	}*/
	//New (untested) code
	while (w > 0) {
		uint16_t transactionWidth = w;
		if (transactionWidth*2 > CONFIG_DRIVER_VSPI_MAX_TRANSFERSIZE) {
			transactionWidth = CONFIG_DRIVER_VSPI_MAX_TRANSFERSIZE/2;
		}
		res = driver_ili9341_set_addr_window(x0, y0, transactionWidth, h);
		if (res != ESP_OK) return res;
		for (uint16_t currentLine = 0; currentLine < h; currentLine++) {
			for (uint16_t i = 0; i<transactionWidth; i++) {
				internalBuffer[i*2+0] = frameBuffer[((x0+i)+(y0+currentLine)*ILI9341_WIDTH)*2+0];
				internalBuffer[i*2+1] = frameBuffer[((x0+i)+(y0+currentLine)*ILI9341_WIDTH)*2+1];
			}
			res = driver_ili9341_send(internalBuffer, transactionWidth*2, true);
			if (res != ESP_OK) return res;
		}
		w -= transactionWidth;
		x0 += transactionWidth;
	}
#endif
	return res;
}

#else
esp_err_t driver_ili9341_init(void) { return ESP_OK; } //Dummy function.
#endif // CONFIG_DRIVER_ILI9341_ENABLE
