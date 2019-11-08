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

#include "include/driver_st7735.h"

#ifdef CONFIG_DRIVER_ST7735_ENABLE

static const char *TAG = "st7735";

uint8_t *internalBuffer; //Internal transfer buffer for doing partial updates

static spi_device_handle_t spi_bus = NULL;

static void driver_st7735_spi_pre_transfer_callback(spi_transaction_t *t)
{
	uint8_t dc_level = *((uint8_t *) t->user);
	gpio_set_level(CONFIG_PIN_NUM_ST7735_DCX, (int) dc_level);
}

const uint8_t st7735_init_data[] = {
    ST7735_FRMCTR1,  3, 0x01, 0x2C, 0x2D,                   // FRMCTR1 frame rate control 1, use by default
    ST7735_FRMCTR2,  3, 0x01, 0x2C, 0x2D,                   // FRMCTR2, Frame Rate Control (In Idle mode/ 8-colors)
    ST7735_FRMCTR3,  6, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D, // FRMCTR3
    ST7735_INVCTR,   1, 0x07,                               // INVCTR display inversion, use by default
    ST7735_DISSET5,  2, 0x15, 0x02,                         // DISSET5
    ST7735_PWCTR1,   3, 0xA2, 0x02, 0x84,                   // PWCTR1 power control 1
    ST7735_PWCTR2,   1, 0xC5,                               // PWCTR2 power control 2
    ST7735_PWCTR3,   2, 0x0A, 0x00,                         // PWCTR3 power control 3
    ST7735_PWCTR4,   2, 0x8A, 0x2A,                         // PWCTR4 (C3h): Power Control 4 (in Idle mode/ 8-colors)
    ST7735_PWCTR5,   2, 0x8A, 0xEE,                         // PWCTR5 (C4h): Power Control 5 (in Partial mode/ full-colors)
    ST7735_VMCTR1,   1, 0x0E,                               // VMCTR vcom control 1
    ST7735_INVOFF,   0,                                     // INVOFF (20h): Display Inversion Off
    ST7735_MADCTL,   1, 0b01100000,                         // MADCTL // enable fake "vertical addressing" mode (for il9163_setBlock() )
    ST7735_COLMOD,   1, 0x05,                               // COLMOD set 16-bit pixel format
    ST7735_GMCTRP1, 16, 0x0F, 0x1A, 0x0F, 0x18, 0x2F, 0x28, 0x20, 0x22, 0x1F, 0x1B, 0x23, 0x37, 0x00, 0x07, 0x02, 0x10,
    ST7735_GMCTRN1, 16, 0x0F, 0x1B, 0x0F, 0x17, 0x33, 0x2C, 0x29, 0x2E, 0x30, 0x30, 0x39, 0x3F, 0x00, 0x07, 0x03, 0x10,
	0x00
};

esp_err_t driver_st7735_send(const uint8_t *data, int len, const uint8_t dc_level)
{
	if (len == 0) return ESP_OK;
	spi_transaction_t t = {
		.length = len * 8,  // transaction length is in bits
		.tx_buffer = data,
		.user = (void *) &dc_level,
	};
	return spi_device_transmit(spi_bus, &t);
}

esp_err_t driver_st7735_receive(uint8_t *data, int len, const uint8_t dc_level)
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

esp_err_t driver_st7735_write_initData(const uint8_t * data)
{
	uint8_t cmd, len;
	while(true) {
		cmd = *data++;
		if(!cmd) return ESP_OK; //END
		len = *data++;
		driver_st7735_send(&cmd, 1, false);
		driver_st7735_send(data, len, true);
		data+=len;
	}
	return ESP_OK;
}

esp_err_t driver_st7735_send_command(uint8_t cmd)
{
	return driver_st7735_send(&cmd, 1, false);
}

esp_err_t driver_st7735_send_u32(uint32_t data)
{
	uint8_t buffer[4];
	buffer[0] = (data>>24)&0xFF;
	buffer[1] = (data>>16)&0xFF;
	buffer[2] = (data>> 8)&0xFF;
	buffer[3] = data      &0xFF;
	return driver_st7735_send(buffer, 4, true);
}

esp_err_t driver_st7735_set_addr_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	uint32_t xa = ((uint32_t)x << 16) | (x+w-1);
	uint32_t ya = ((uint32_t)y << 16) | (y+h-1);
	esp_err_t res;
	res = driver_st7735_send_command(ST7735_CASET);
	if (res != ESP_OK) return res;
	res = driver_st7735_send_u32(xa);
	if (res != ESP_OK) return res;
	res = driver_st7735_send_command(ST7735_RASET);
	if (res != ESP_OK) return res;
	res = driver_st7735_send_u32(ya);
	if (res != ESP_OK) return res;
	res = driver_st7735_send_command(ST7735_RAMWR);
	return res;
	return ESP_OK;
}

esp_err_t driver_st7735_reset(void) {
	#if CONFIG_PIN_NUM_ST7735_RESET >= 0
	esp_err_t res = gpio_set_level(CONFIG_PIN_NUM_ST7735_RESET, false);
	if (res != ESP_OK) return res;
	ets_delay_us(200000);
	res = gpio_set_level(CONFIG_PIN_NUM_ST7735_RESET, true);
	if (res != ESP_OK) return res;
	#endif
	ets_delay_us(200000);
	return ESP_OK;
}

esp_err_t driver_st7735_set_backlight(bool state)
{
	#if CONFIG_PIN_NUM_ST7735_BACKLIGHT >= 0
		return gpio_set_level(CONFIG_PIN_NUM_ST7735_BACKLIGHT, !state);
	#else
		return ESP_OK;
	#endif
}

esp_err_t driver_st7735_set_sleep(bool state)
{
	esp_err_t res;
	if (state) {
		res = driver_st7735_send_command(ST7735_SLPIN);
		if (res != ESP_OK) return res;
	} else {
		res = driver_st7735_send_command(ST7735_SLPOUT);
		if (res != ESP_OK) return res;
	}
	vTaskDelay(200 / portTICK_PERIOD_MS);
	return res;
}

esp_err_t driver_st7735_init(void)
{
	static bool driver_st7735_init_done = false;
	if (driver_st7735_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	esp_err_t res;
	
	//Initialize backlight GPIO pin
	#if CONFIG_PIN_NUM_ST7735_BACKLIGHT>=0
		res = gpio_set_direction(CONFIG_PIN_NUM_ST7735_BACKLIGHT, GPIO_MODE_OUTPUT);
		if (res != ESP_OK) return res;
	#endif

	//Turn off backlight
	res = driver_st7735_set_backlight(false);
	if (res != ESP_OK) return res;

	//Allocate partial update buffer
	internalBuffer = heap_caps_malloc(CONFIG_DRIVER_VSPI_MAX_TRANSFERSIZE, MALLOC_CAP_8BIT);
	if (!internalBuffer) return ESP_FAIL;
	
	//Initialize reset GPIO pin
	#if CONFIG_PIN_NUM_ST7735_RESET >= 0
	res = gpio_set_direction(CONFIG_PIN_NUM_ST7735_RESET, GPIO_MODE_OUTPUT);
	if (res != ESP_OK) return res;
	#endif

	//Initialize data/clock select GPIO pin
	res = gpio_set_direction(CONFIG_PIN_NUM_ST7735_DCX, GPIO_MODE_OUTPUT);
	if (res != ESP_OK) return res;
	
	static const spi_device_interface_config_t devcfg = {
		.clock_speed_hz = 4 * 1000 * 1000,
		.mode           = 0,  // SPI mode 0
		.spics_io_num   = CONFIG_PIN_NUM_ST7735_CS,
		.queue_size     = 1,
		.flags          = (SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE),//SPI_DEVICE_HALFDUPLEX,
		.pre_cb         = driver_st7735_spi_pre_transfer_callback, // Specify pre-transfer callback to handle D/C line
	};
	res = spi_bus_add_device(VSPI_HOST, &devcfg, &spi_bus);
	if (res != ESP_OK) return res;

	//Reset the LCD display
	res = driver_st7735_reset();
	if (res != ESP_OK) return res;
	
	//Wake-up display
	res = driver_st7735_set_sleep(false);
	if (res != ESP_OK) return res;
	
	ets_delay_us(1000);

	//Send the initialization data to the LCD display
	res = driver_st7735_write_initData(st7735_init_data);
	if (res != ESP_OK) return res;
	
	ets_delay_us(1000);
	
	//
	res = driver_st7735_send_command(ST7735_DISPON);
	if (res != ESP_OK) return res;
	
	ets_delay_us(1000);
	
	//
	res = driver_st7735_send_command(ST7735_NORON);
	if (res != ESP_OK) return res;
	
	//Turn on backlight
	res = driver_st7735_set_backlight(true);
	if (res != ESP_OK) return res;

	driver_st7735_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

esp_err_t driver_st7735_write(const uint8_t *buffer)
{
	return driver_st7735_write_partial(buffer, 0, 0, ST7735_WIDTH-1, ST7735_HEIGHT-1);
}

esp_err_t driver_st7735_write_partial_direct(const uint8_t *buffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{ //Without conversion
	if (x0 > x1) return ESP_FAIL;
	if (y0 > y1) return ESP_FAIL;
	uint16_t w = x1-x0;
	uint16_t h = y1-y0;
	esp_err_t res = driver_st7735_set_addr_window(x0, y0, w, h);
	if (res != ESP_OK) return res;
	res = driver_st7735_send(buffer, w*h*2, true);
	return res;
}

esp_err_t driver_st7735_write_partial(const uint8_t *frameBuffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{ //With conversion from framebuffer
	esp_err_t res = ESP_OK;
	if (x0 > x1) return ESP_FAIL;
	if (y0 > y1) return ESP_FAIL;
	if (x1 >= ST7735_WIDTH)  x1 = ST7735_WIDTH-1;
	if (y1 >= ST7735_HEIGHT) y1 = ST7735_HEIGHT-1;
	
	uint16_t w = x1-x0+1;
	uint16_t h = y1-y0+1;
	
	printf("Driver ST7735 write @ %d, %d with width %d and height %d\n", x0, y0, w, h);

	//if (w >= ST7735_WIDTH) {
		while (h > 0) {
			uint16_t lines = h;
			if (lines > 1) lines = 1;
			esp_err_t res = driver_st7735_set_addr_window(0, y0+ST7735_OFFSET_Y, ST7735_WIDTH, lines);
			if (res != ESP_OK) break;
			res = driver_st7735_send(frameBuffer+(y0*ST7735_WIDTH)*2, ST7735_WIDTH*lines*2, true);
			if (res != ESP_OK) break;
			y0 += lines;
			h -= lines;
		}
	/*} else {
		while (h > 0) {
			uint16_t lines = h;
			if (w*2*lines > CONFIG_DRIVER_VSPI_MAX_TRANSFERSIZE) {
				lines = CONFIG_DRIVER_VSPI_MAX_TRANSFERSIZE/(w*2);
			}
			for (uint16_t y = 0; y < lines; y++) {
				uint32_t internalBufferOffset = y*w*2; //Current line * width * 2 (because 16-bit per pixel)
				uint32_t frameBufferOffset    = (x0+(y0+y)*ST7735_WIDTH)*2;
				memcpy(internalBuffer+internalBufferOffset, frameBuffer+frameBufferOffset, w*2);
				res = driver_st7735_set_addr_window(x0, y0, w, lines);
				if (res != ESP_OK) return res;
				res = driver_st7735_send(internalBuffer, w*lines*2, true);
				if (res != ESP_OK) return res;
			}
			h -= lines;
			y0 += lines;
		}
	}*/
	return res;
}

#else
esp_err_t driver_st7735_init(void) { return ESP_OK; } //Dummy function.
#endif // CONFIG_DRIVER_ST7735_ENABLE
