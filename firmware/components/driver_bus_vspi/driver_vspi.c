#include <sdkconfig.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <esp_err.h>

#include <driver/spi_master.h>

#include "include/driver_vspi.h"

#ifdef CONFIG_DRIVER_VSPI_ENABLE

#define TAG "vspi"

esp_err_t driver_vspi_init(void)
{
	static bool driver_vspi_init_done = false;
	if (driver_vspi_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	static const spi_bus_config_t buscfg = {
		.mosi_io_num     = CONFIG_PIN_NUM_VSPI_MOSI,
		.miso_io_num     = CONFIG_PIN_NUM_VSPI_MISO,
		.sclk_io_num     = CONFIG_PIN_NUM_VSPI_CLK,
		.quadwp_io_num   = CONFIG_PIN_NUM_VSPI_WP,
		.quadhd_io_num   = CONFIG_PIN_NUM_VSPI_HD,
		.max_transfer_sz = CONFIG_DRIVER_VSPI_MAX_TRANSFERSIZE,
	};
	
	esp_err_t res = spi_bus_initialize(VSPI_HOST, &buscfg, CONFIG_DRIVER_VSPI_DMA_CHANNEL);
	if (res != ESP_OK) return res;
	
	driver_vspi_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

#else // CONFIG_DRIVER_VSPI_ENABLE
esp_err_t driver_vspi_init(void) { return ESP_OK; } // Dummy function, leave empty!
#endif
