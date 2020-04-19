#include <sdkconfig.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <esp_err.h>

#include <driver/i2c.h>
#include <freertos/task.h>

#include "include/driver_dotflipper.h"

#ifdef CONFIG_DRIVER_DOTFLIPPER_ENABLE

#define TAG "DOTFLIPPER"


uint8_t __current_col = 0;
uint8_t __current_row = 0;

bool __busy           = false;
bool __queue          = false;
bool __state_unknown  = false;

uint8_t* stateCurrent = NULL;
const uint8_t* stateTarget  = NULL;

xSemaphoreHandle driver_dotflipper_refresh_trigger = NULL;

inline bool _get_pixel(uint8_t col, uint8_t row, const uint8_t* buffer)
{
	//1BPP horizontal
	//uint32_t position = (row * (DOTFLIPPER_WIDTH / 8)) + ((col + (mod * CONFIG_DOTFLIPPER_COLS)) / 8);
	//uint8_t  bit      = ((col + (mod * CONFIG_DOTFLIPPER_COLS))) % 8;

	uint16_t x = col;
	uint16_t y = row;
	
	uint32_t position = ( (y / 8) * DOTFLIPPER_WIDTH) + x;
	uint8_t  bit      = y % 8;
	return !((buffer[position] >> bit)&0x01);
}

esp_err_t driver_dotflipper_set_pixel(uint8_t x, uint8_t y, bool color)
{
	x %= DOTFLIPPER_WIDTH * DOTFLIPPER_PANELWIDTH;
	y %= DOTFLIPPER_HEIGHT * DOTFLIPPER_PANELHEIGHT;

	// Quick mafs
	// Find out what panel we're on
	uint8_t xpanelcoordinate = x / DOTFLIPPER_PANELWIDTH;
	uint8_t ypanelcoordinate = y / DOTFLIPPER_PANELHEIGHT;
	uint8_t panelnumber = xpanelcoordinate + (ypanelcoordinate * DOTFLIPPER_WIDTH);

	// Determine which drivers we want to speak to
	uint8_t columndriveraddress = 0x40 + ((panelnumber * 4) + ((x % DOTFLIPPER_PANELWIDTH)  / 8));
	uint8_t rowdriveraddress    = 0x60 + ((panelnumber * 2) + ((y % DOTFLIPPER_PANELHEIGHT) / 8));

	uint8_t column = x % 8;
	uint8_t columnregisteroffset = (color) ? 0x07 : 0x27;
	uint8_t columnregister = 4 * column + columnregisteroffset;

	uint8_t row = y % 8;
	uint8_t rowregisteroffset = (color) ? 0x27 : 0x07;
	uint8_t rowregister =  4 * row + rowregisteroffset;

	// Powerup!

	driver_i2c_write_reg(rowdriveraddress,    rowregister          , 0x10);
	driver_i2c_write_reg(columndriveraddress, columnregister       , 0x10);

	// Need a delay here!!!
	ets_delay_us(DOTFLIPPER_SINGLEPANEL_DELAY);

	// And powerdown!
	driver_i2c_write_reg(rowdriveraddress,    rowregister          , 0x00);
	driver_i2c_write_reg(columndriveraddress, columnregister       , 0x00);
    
	// Need a delay here!!!
	ets_delay_us(DOTFLIPPER_SINGLEPANEL_DELAY);
	
	return ESP_OK;
}

void driver_dotflipper_refresh_task(void *arg)
{
	while (1) { //This function runs as a FreeRTOS task, that is why it has it's own "main" loop.
		if (xSemaphoreTake(driver_dotflipper_refresh_trigger, portMAX_DELAY)) { //This check lets FreeRTOS pause this task until we get permission to access the "shared resource", which in this case is the refresh trigger
			if (__current_col >= DOTFLIPPER_WIDTH) {
				__current_col = 0;
				__current_row++;
			}
			
			if (__current_row >= DOTFLIPPER_HEIGHT) {
				//Done
				memcpy(stateCurrent, stateTarget, DOTFLIPPER_BUFFER_SIZE);
				__state_unknown = false;
				if (__queue) {
					// Start over
					__queue = false;
					__current_col = 0;
					__current_row = 0;
				
					xSemaphoreGive(driver_dotflipper_refresh_trigger); //Continue
				} else {
					// Stop
					//printf("refresh: done\n");
					__busy = false;
				}
			} else {
				bool currentValue = _get_pixel( __current_col, __current_row, stateCurrent);
				bool targetValue  = _get_pixel( __current_col, __current_row, stateTarget);
				
				if ((currentValue == targetValue) && (!__state_unknown)) {
					//printf("%d, %d, %d: -\n", __current_col, __current_row, __current_mod);
					__current_col++; //Go to the next pixel
					xSemaphoreGive(driver_dotflipper_refresh_trigger); //Continue, give ourselves the refresh trigger
				} else {
					//printf("%d, %d, %d: %d\n", __current_col, __current_row, __current_mod, targetValue);
					driver_dotflipper_set_pixel(__current_col, __current_row, targetValue);
					__current_col++; //Go to the next pixel
					//Note: once the SPI transaction is completed the post transfer call back will give us back the refresh trigger semaphore, this task will thus wait for the end of the transfer.
				}
			}
		}
	}
}

esp_err_t driver_dotflipper_write(const uint8_t *buffer)
{
	stateTarget = buffer;
	if (__busy) {
		//If the display is already being refreshed then we tell the driver to start over once done
		__queue = true;
		printf("Flipdot is busy, queued refresh cycle\n");
	} else {
		//If the display is idle then we start the refresh cycle
		__busy = true;
		__queue = false;
		__current_col = 0;
		__current_row = 0;
		xSemaphoreGive(driver_dotflipper_refresh_trigger);
	}
	return ESP_OK;
}

//static void driver_dotflipper_post_transfer_callback(spi_transaction_t *t)
//{
//	gpio_set_level(CONFIG_PIN_NUM_DOTFLIPPER_FIRE, true);
//	ets_delay_us(250);
//	gpio_set_level(CONFIG_PIN_NUM_DOTFLIPPER_FIRE, false);
//	xSemaphoreGiveFromISR(driver_dotflipper_refresh_trigger, NULL);
//}

void driver_dotflipper_everythingoff(uint8_t address) {
  // Write to the ALL_LED registers to switch them all off.
  driver_i2c_write_reg(address, 0xFA, 0x00);
  driver_i2c_write_reg(address, 0xFB, 0x00);
  driver_i2c_write_reg(address, 0xFC, 0x00);
  driver_i2c_write_reg(address, 0xFD, 0x00);
}

void driver_dotflipper_resetPCA9685(uint8_t adress){
    driver_i2c_write_reg(adress, 0x00, 0x80);       // 
    driver_i2c_write_reg(adress, 0x00, 0b00000001); // Awake and responds to 'all call'
    driver_i2c_write_reg(adress, 0x01, 0b00000100); //
    driver_i2c_write_reg(adress, 0x00, 0b00010001); // Sleep mode and responds to 'all call'
    driver_i2c_write_reg(adress, 0xFE, 0x03);       // Set PWM frequency to 1526hz (maximum), formula is round( (osc_clock / ( 4096*update_rate ) ) - 1 ), so round ( ( 25mhz / (4096*1526) ) - 1 ) = 0x03
    driver_i2c_write_reg(adress, 0x00, 0b00000001); // Awake and responds to 'all call'
    driver_dotflipper_everythingoff(adress);
}

static inline esp_err_t _init(void)
{
	// tzt hier naartoe omkatten:
	// if ( driver_i2c_write_reg(CONFIG_I2C_ADDR_MPU6050, MPU6050_SMPRT_DIV,    0x00) != ESP_OK ) return ESP_FAIL;
	
	for (uint8_t x = 0x40; x < 0x40+(DOTFLIPPER_NUMPANELS*4); x++) {
    	driver_dotflipper_resetPCA9685(x);
  	}
    for (uint8_t y = 0x60; y < 0x60+(DOTFLIPPER_NUMPANELS*2); y++) {
    	driver_dotflipper_resetPCA9685(y);
  	}
	return ESP_OK;
}
	
esp_err_t driver_dotflipper_init(void)
{
	static bool driver_dotflipper_init_done = false;
	if (driver_dotflipper_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	esp_err_t res = _init(); //Call the real init function
	
	if (res != ESP_OK) return res;
	
	//Allocate buffer for current state
	stateCurrent = malloc(DOTFLIPPER_BUFFER_SIZE);
	if (!stateCurrent) return ESP_FAIL;
	memset(stateCurrent, 0x00, DOTFLIPPER_BUFFER_SIZE);
	__state_unknown = true;
	
	//Initialize semaphore for refresh task
	driver_dotflipper_refresh_trigger = xSemaphoreCreateBinary();
	if (driver_dotflipper_refresh_trigger == NULL) return ESP_ERR_NO_MEM;
	
	//Create the refresh task
	xTaskCreate(&driver_dotflipper_refresh_task, "Flipdot display refresh task", 4096, NULL, 10, NULL);
	
	driver_dotflipper_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

#else // CONFIG_DRIVER_DOTFLIPPER_ENABLE
esp_err_t driver_dotflipper_init(void) { return ESP_OK; } // Dummy function, leave empty!
#endif
