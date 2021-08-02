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

#include "include/driver_st7789v.h"

#ifdef CONFIG_DRIVER_ST7789V_ENABLE

static const char *TAG = "st7789v";

uint8_t *internalBuffer; //Internal transfer buffer for doing partial updates

static spi_device_handle_t spi_bus = NULL;

static void driver_st7789v_spi_pre_transfer_callback(spi_transaction_t *t)
{
	uint8_t dc_level = *((uint8_t *) t->user);
	gpio_set_level(CONFIG_PIN_NUM_ST7789V_DCX, (int) dc_level);
}

const uint8_t st7789v_init_data[] = {
    ST7789V_COLMOD,   1, 0x55,                               // 16-bit color mode, 65K
    ST7789V_MADCTL,   1, 0b01100000,                         // 
#ifdef CONFIG_DRIVER_ST7789V_COLOR_INVERT
	ST7789V_INVON,    0,                                     // Enable inversion (Adafruit does this)
#endif
	0x00
};

esp_err_t driver_st7789v_send(const uint8_t *data, int len, const uint8_t dc_level)
{
	if (len == 0) return ESP_OK;
	spi_transaction_t t = {
		.length = len * 8,  // transaction length is in bits
		.tx_buffer = data,
		.user = (void *) &dc_level,
	};
	return spi_device_transmit(spi_bus, &t);
}

esp_err_t driver_st7789v_receive(uint8_t *data, int len, const uint8_t dc_level)
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

esp_err_t driver_st7789v_write_initData(const uint8_t * data)
{
	uint8_t cmd, len;
	while(true) {
		cmd = *data++;
		if(!cmd) return ESP_OK; //END
		len = *data++;
		driver_st7789v_send(&cmd, 1, false);
		driver_st7789v_send(data, len, true);
		data+=len;
	}
	return ESP_OK;
}

esp_err_t driver_st7789v_send_command(uint8_t cmd)
{
	return driver_st7789v_send(&cmd, 1, false);
}

esp_err_t driver_st7789v_send_u32(uint32_t data)
{
	uint8_t buffer[4];
	buffer[0] = (data>>24)&0xFF;
	buffer[1] = (data>>16)&0xFF;
	buffer[2] = (data>> 8)&0xFF;
	buffer[3] = data      &0xFF;
	return driver_st7789v_send(buffer, 4, true);
}

esp_err_t driver_st7789v_set_addr_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	uint32_t xa = ((uint32_t)x << 16) | (x+w-1);
	uint32_t ya = ((uint32_t)y << 16) | (y+h-1);
	esp_err_t res;
	res = driver_st7789v_send_command(ST7789V_CASET);
	if (res != ESP_OK) return res;
	res = driver_st7789v_send_u32(xa);
	if (res != ESP_OK) return res;
	res = driver_st7789v_send_command(ST7789V_RASET);
	if (res != ESP_OK) return res;
	res = driver_st7789v_send_u32(ya);
	if (res != ESP_OK) return res;
	res = driver_st7789v_send_command(ST7789V_RAMWR);
	return res;
	return ESP_OK;
}

esp_err_t driver_st7789v_reset(void) {
	#if CONFIG_PIN_NUM_ST7789V_RESET >= 0
	esp_err_t res = gpio_set_level(CONFIG_PIN_NUM_ST7789V_RESET, false);
	if (res != ESP_OK) return res;
	ets_delay_us(200000);
	res = gpio_set_level(CONFIG_PIN_NUM_ST7789V_RESET, true);
	if (res != ESP_OK) return res;
	#else //Reset pin not connected, use software reset
	esp_err_t res = driver_st7789v_send_command(ST7789V_SWRESET);
	if (res != ESP_OK) return res;
	#endif
	ets_delay_us(200000);
	return ESP_OK;
}

esp_err_t driver_st7789v_set_backlight(bool state)
{
	#if CONFIG_PIN_NUM_ST7789V_BACKLIGHT >= 0
		#ifdef CONFIG_DRIVER_ST7789V_BACKLIGHT_INVERT
			state = !state;
		#endif
		return gpio_set_level(CONFIG_PIN_NUM_ST7789V_BACKLIGHT, state);
	#else
		return ESP_OK;
	#endif
}

esp_err_t driver_st7789v_set_sleep(bool state)
{
	esp_err_t res;
	if (state) {
		res = driver_st7789v_send_command(ST7789V_SLPIN);
		if (res != ESP_OK) return res;
	} else {
		res = driver_st7789v_send_command(ST7789V_SLPOUT);
		if (res != ESP_OK) return res;
	}
	vTaskDelay(200 / portTICK_PERIOD_MS);
	return res;
}

esp_err_t driver_st7789v_init(void)
{
	static bool driver_st7789v_init_done = false;
	if (driver_st7789v_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	esp_err_t res;
	
	//Initialize backlight GPIO pin
	#if CONFIG_PIN_NUM_ST7789V_BACKLIGHT>=0
		res = gpio_set_direction(CONFIG_PIN_NUM_ST7789V_BACKLIGHT, GPIO_MODE_OUTPUT);
		if (res != ESP_OK) return res;
	#endif

	//Turn off backlight
	res = driver_st7789v_set_backlight(false);
	if (res != ESP_OK) return res;

	//Allocate partial update buffer
	internalBuffer = heap_caps_malloc(CONFIG_DRIVER_VSPI_MAX_TRANSFERSIZE, MALLOC_CAP_8BIT);
	if (!internalBuffer) return ESP_FAIL;
	
	//Initialize reset GPIO pin
	#if CONFIG_PIN_NUM_ST7789V_RESET >= 0
	res = gpio_set_direction(CONFIG_PIN_NUM_ST7789V_RESET, GPIO_MODE_OUTPUT);
	if (res != ESP_OK) return res;
	#endif

	//Initialize data/clock select GPIO pin
	res = gpio_set_direction(CONFIG_PIN_NUM_ST7789V_DCX, GPIO_MODE_OUTPUT);
	if (res != ESP_OK) return res;
	
	static const spi_device_interface_config_t devcfg = {
		.clock_speed_hz = CONFIG_DRIVER_ST7789V_SPI_SPEED,
		.mode           = 0,  // SPI mode 0
		.spics_io_num   = CONFIG_PIN_NUM_ST7789V_CS,
		.queue_size     = 1,
		.flags          = (SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE),//SPI_DEVICE_HALFDUPLEX,
		.pre_cb         = driver_st7789v_spi_pre_transfer_callback, // Specify pre-transfer callback to handle D/C line
	};
	res = spi_bus_add_device(VSPI_HOST, &devcfg, &spi_bus);
	if (res != ESP_OK) return res;

	//Reset the LCD display
	res = driver_st7789v_reset();
	if (res != ESP_OK) return res;
	
	//Wake-up display
	res = driver_st7789v_set_sleep(false);
	if (res != ESP_OK) return res;
	
	ets_delay_us(1000);

	//Send the initialization data to the LCD display
	res = driver_st7789v_write_initData(st7789v_init_data);
	if (res != ESP_OK) return res;
	
	ets_delay_us(1000);
	
	//
	res = driver_st7789v_send_command(ST7789V_DISPON);
	if (res != ESP_OK) return res;
	
	ets_delay_us(1000);
	
	//
	res = driver_st7789v_send_command(ST7789V_NORON);
	if (res != ESP_OK) return res;
	
	//Backlight
#ifdef CONFIG_DRIVER_ST7789V_BACKLIGHT_AT_BOOT
	res = driver_st7789v_set_backlight(true);
#else
	res = driver_st7789v_set_backlight(false);
#endif
	if (res != ESP_OK) return res;

	driver_st7789v_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

esp_err_t driver_st7789v_write(const uint8_t *buffer)
{
	return driver_st7789v_write_partial(buffer, 0, 0, ST7789V_WIDTH-1, ST7789V_HEIGHT-1);
}

esp_err_t driver_st7789v_write_partial_direct(const uint8_t *buffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{ //Without conversion
	if (x0 > x1) return ESP_FAIL;
	if (y0 > y1) return ESP_FAIL;
	uint16_t w = x1-x0;
	uint16_t h = y1-y0;
	esp_err_t res = driver_st7789v_set_addr_window(x0, y0, w, h);
	if (res != ESP_OK) return res;
	res = driver_st7789v_send(buffer, w*h*2, true);
	return res;
}

esp_err_t driver_st7789v_write_partial(const uint8_t *frameBuffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{ //With conversion from framebuffer
	esp_err_t res = ESP_OK;
	if (x0 > x1) return ESP_FAIL;
	if (y0 > y1) return ESP_FAIL;
	if (x1 >= ST7789V_WIDTH)  x1 = ST7789V_WIDTH-1;
	if (y1 >= ST7789V_HEIGHT) y1 = ST7789V_HEIGHT-1;
	
	uint16_t w = x1-x0+1;
	uint16_t h = y1-y0+1;
    
#if CONFIG_DRIVER_ST7789V_8C
	while (w > 0) {
		uint16_t transactionWidth = w;
		if (transactionWidth*2 > CONFIG_DRIVER_VSPI_MAX_TRANSFERSIZE) {
			transactionWidth = CONFIG_DRIVER_VSPI_MAX_TRANSFERSIZE/2;
		}
		res = driver_st7789v_set_addr_window(x0+ST7789V_OFFSET_X, y0+ST7789V_OFFSET_Y, transactionWidth, h);
		if (res != ESP_OK) return res;
		for (uint16_t currentLine = 0; currentLine < h; currentLine++) {
			for (uint16_t i = 0; i<transactionWidth; i++) {
				uint8_t color8 = frameBuffer[x0+i+(y0+currentLine)*ST7789V_WIDTH];
				uint8_t r = color8 & 0x07;
				uint8_t g = (color8>>3) & 0x07;
				uint8_t b = color8 >> 6;
				internalBuffer[i*2+0] = g | (r << 5);
				internalBuffer[i*2+1] = (b << 3);
			}
			res = driver_st7789v_send(internalBuffer, transactionWidth*2, true);
			if (res != ESP_OK) return res;
		}
		w -= transactionWidth;
		x0 += transactionWidth;
	}
#else
	while (w > 0) {
		uint16_t transactionWidth = w;
		if (transactionWidth*2 > CONFIG_DRIVER_VSPI_MAX_TRANSFERSIZE) {
			transactionWidth = CONFIG_DRIVER_VSPI_MAX_TRANSFERSIZE/2;
		}
		res = driver_st7789v_set_addr_window(x0+ST7789V_OFFSET_X, y0+ST7789V_OFFSET_Y, transactionWidth, h);
		if (res != ESP_OK) return res;
		for (uint16_t currentLine = 0; currentLine < h; currentLine++) {
			for (uint16_t i = 0; i<transactionWidth; i++) {
				internalBuffer[i*2+0] = frameBuffer[((x0+i)+(y0+currentLine)*ST7789V_WIDTH)*2+0];
				internalBuffer[i*2+1] = frameBuffer[((x0+i)+(y0+currentLine)*ST7789V_WIDTH)*2+1];
			}
			res = driver_st7789v_send(internalBuffer, transactionWidth*2, true);
			if (res != ESP_OK) return res;
		}
		w -= transactionWidth;
		x0 += transactionWidth;
	}
#endif
	return res;
}

#else
esp_err_t driver_st7789v_init(void) { return ESP_OK; } //Dummy function.
#endif // CONFIG_DRIVER_ST7789V_ENABLE
