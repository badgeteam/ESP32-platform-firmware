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
#include <nvs_flash.h>
#include <nvs.h>
#include <driver/gpio.h>

#include "include/driver_openaars.h"

#ifdef CONFIG_DRIVER_OPENAARS_ENABLE

static const char *TAG = "openaars";
xSemaphoreHandle driver_openaars_intreg = NULL;          // Mutex for accessing the interrupt flag register
xSemaphoreHandle driver_openaars_mux = NULL;          // Mutex for accessing driver_openaars_state, driver_openaars_handlers, etc..
xSemaphoreHandle driver_openaars_intr_trigger = NULL; // semaphore to trigger MPR121 interrupt handling

// Interrupt callbacks
driver_openaars_intr_t driver_openaars_handlers[2] = { NULL,NULL };
void*                driver_openaars_arg[2]      = { NULL,NULL };

// uint8_t *internalBuffer; //Internal transfer buffer for doing partial updates

static spi_device_handle_t spi_bus = NULL;

uint32_t interrupt_reg = 0;                           // Interrupt flag register

// Interrupt handler FPGA

// Interrupt task to initiate the interrupt sequence once the semaphore is given
void driver_openaars_intr_task(void *arg)
{
  uint32_t loc_interrupt_reg = 0; 

	while (1) {
    // Process the interrupts
    
		if (xSemaphoreTake(driver_openaars_intr_trigger, portMAX_DELAY)) {
      // Update the flag register
      xSemaphoreTake(driver_openaars_intreg, portMAX_DELAY);
      loc_interrupt_reg = interrupt_reg;
      xSemaphoreGive(driver_openaars_intreg);

      // If flag set, call the FPGA routine
      for (uint32_t int_nr = 0; int_nr < INT_OPENAARS_LAST_NR; int_nr++) {
        if ((loc_interrupt_reg && (uint32_t)1<<int_nr) != 0) {
          if(xSemaphoreTake(driver_openaars_mux, portMAX_DELAY)) {
            driver_openaars_intr_t handler = driver_openaars_handlers[int_nr];
            void *arg = driver_openaars_arg[int_nr];
            xSemaphoreGive(driver_openaars_mux);
            if (handler != NULL) handler(arg, 0);
          }
        }
      }
    }
  }
}

// The interrupt handler that is not allowed to do IO, so we have it release a 
void driver_openaars_intr_handler(void *arg)
{ 
  int intr_id = (int) arg;
  /* in interrupt handler */
// 	if (
//       gpio_get_level(CONFIG_PIN_NUM_OPENAARS_FPGA_INT) == 0 ||
//       gpio_get_level(CONFIG_PIN_NUM_OPENAARS_ADV_INT) == 0
//       ) {
    // Add the flag on the interrupt map
    xSemaphoreTakeFromISR(driver_openaars_intreg, NULL);
    interrupt_reg |= (uint32_t)1<<intr_id;
    xSemaphoreGiveFromISR(driver_openaars_intreg, NULL);

    // Unlock the thread the interfaces with the frontend
	  xSemaphoreGiveFromISR(driver_openaars_intr_trigger, NULL);
//	}
}
  

// Register the interrupt handling routines from Python
void driver_openaars_set_interrupt_handler(uint8_t pin, driver_openaars_intr_t handler, void *arg)
{
	if (driver_openaars_mux == NULL) { // allow setting handlers when driver_openaars is not initialized yet.
		driver_openaars_handlers[pin] = handler;
		driver_openaars_arg[pin] = arg;
	} else {
		xSemaphoreTake(driver_openaars_mux, portMAX_DELAY);
		driver_openaars_handlers[pin] = handler;
		driver_openaars_arg[pin] = arg;
		xSemaphoreGive(driver_openaars_mux);
	}
}

// SPI transfers

static void driver_openaars_spi_pre_transfer_callback(spi_transaction_t *t)
{
  //uint8_t dc_level = *((uint8_t *) t->user);
  //gpio_set_level(CONFIG_PIN_NUM_ILI9341_DCX, (int) dc_level);
}

const uint8_t openaars_init_data[] = {};

esp_err_t driver_openaars_send(const uint8_t *data, int len, const uint8_t dc_level)
{
	if (len == 0) return ESP_OK;
	spi_transaction_t t = {
		.length = len * 8,  // transaction length is in bits
		.tx_buffer = data,
		.user = (void *) &dc_level,
	};
	return spi_device_transmit(spi_bus, &t);
}

esp_err_t driver_openaars_receive(uint8_t *data, int len, const uint8_t dc_level)
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

esp_err_t driver_openaars_reset(void) {
	return ESP_OK;
}

esp_err_t driver_openaars_send_command(uint8_t cmd)
{
	return driver_openaars_send(&cmd, 1, false);
}

esp_err_t driver_openaars_init(void)
{
	esp_err_t res;

	static bool driver_openaars_init_done = false;
	if (driver_openaars_init_done) return ESP_OK;
	ESP_LOGI(TAG, "init called");

  // Create the semaphores
	ESP_LOGI(TAG, "Setup interrupt");

  // FPGA
	driver_openaars_mux = xSemaphoreCreateMutex();
	if (driver_openaars_mux == NULL) return ESP_ERR_NO_MEM;
  // Interrupt flag register Mutex
	driver_openaars_intreg = xSemaphoreCreateMutex();
	if (driver_openaars_intreg == NULL) return ESP_ERR_NO_MEM;

	driver_openaars_intr_trigger = xSemaphoreCreateBinary();
	if (driver_openaars_intr_trigger == NULL) return ESP_ERR_NO_MEM;

  // Register FPGA INT PIN HANDLER
	res = gpio_isr_handler_add(CONFIG_PIN_NUM_OPENAARS_FPGA_INT, 
                             driver_openaars_intr_handler, 
                             (void*) INT_OPENAARS_FPGA_NR);
	if (res != ESP_OK) return res;
  // Register ADV INT PIN HANDLER
	res = gpio_isr_handler_add(CONFIG_PIN_NUM_OPENAARS_ADV_INT, 
                             driver_openaars_intr_handler, 
                             (void*) INT_OPENAARS_ADV_NR);
	if (res != ESP_OK) return res;
	ESP_LOGI(TAG, "Setup IO port");

  // Interrupt pin config
	gpio_config_t io_conf = (gpio_config_t) {
		.intr_type    = GPIO_INTR_NEGEDGE,
		.mode         = GPIO_MODE_INPUT,
		.pin_bit_mask = 1LL << CONFIG_PIN_NUM_OPENAARS_FPGA_INT || 1LL << CONFIG_PIN_NUM_OPENAARS_ADV_INT,
		.pull_down_en = 0,
		.pull_up_en   = 1,
	};

	ESP_LOGI(TAG, "Setup IO port NO INT.");
	res = gpio_config(&io_conf);
	if (res != ESP_OK) return res;
	ESP_LOGI(TAG, "Setup IO port done.");

  xTaskCreate(&driver_openaars_intr_task, "AARS FPGA interrupt task", 4096, NULL, 10, NULL);
	xSemaphoreGive(driver_openaars_intr_trigger);
  ESP_LOGI(TAG, "Setup Task done.");

	// Allocate partial update buffer
	// internalBuffer = heap_caps_malloc(CONFIG_DRIVER_VSPI_MAX_TRANSFERSIZE, MALLOC_CAP_8BIT);
	// if (!internalBuffer) return ESP_FAIL;
	
	ESP_LOGD(TAG, "Initialize SPI");
	static const spi_device_interface_config_t devcfg = {
		.clock_speed_hz = 40 * 1000 * 1000,                           // TODO when the V2 board is here up to 80MHz
		.mode           = 0,                                          // SPI mode 0
		.spics_io_num   = CONFIG_PIN_NUM_OPENAARS_CS,
		.queue_size     = 1,
		.flags          = (SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE), // SPI_DEVICE_HALFDUPLEX,
		.pre_cb         = driver_openaars_spi_pre_transfer_callback,  // Specify pre-transfer callback to handle D/C line
	};
	res = spi_bus_add_device(VSPI_HOST, &devcfg, &spi_bus);
	if (res != ESP_OK) return res;

	// Reset the FPGA state
	res = driver_openaars_reset();
	if (res != ESP_OK) return res;

  // Init is done
	driver_openaars_init_done = true;
	ESP_LOGI(TAG, "init done");
	return ESP_OK;
}

#else
esp_err_t driver_openaars_init(void) { return ESP_OK; } //Dummy function.
#endif // CONFIG_DRIVER_ILI9341_ENABLE
