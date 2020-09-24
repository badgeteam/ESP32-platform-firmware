#include <sdkconfig.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <esp_err.h>

#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

#include "include/driver_sdcard.h"

#include "driver_mpr121.h"

#ifdef CONFIG_DRIVER_SDCARD_ENABLE

#define TAG "sdcard"

//SDMMC HOST peripheral pin mapping (note: these are bound by the chip and can NOT be changed!)
#define SDMMC_HOST_PIN_CMD 15
#define SDMMC_HOST_PIN_CLK 14
#define SDMMC_HOST_PIN_D0  2
#define SDMMC_HOST_PIN_D1  4
#define SDMMC_HOST_PIN_D2  12
#define SDMMC_HOST_PIN_D3  13

static bool sdcard_is_mounted = false;

bool driver_sdcard_is_mounted() {
	return sdcard_is_mounted;
}

esp_err_t driver_sdcard_unmount() {
	if (!sdcard_is_mounted) return ESP_OK; //Not mounted
	esp_err_t res = esp_vfs_fat_sdmmc_unmount();
	if (res != ESP_OK) return res;
	#ifdef CONFIG_DRIVER_SDCARD_MODE_SPI
		res = sdspi_host_deinit();
		if (res != ESP_OK) return res;
	#endif
	sdcard_is_mounted = false;
	return ESP_OK;
}

esp_err_t driver_sdcard_mount(const char* mount_point, bool format_if_mount_failed) {
	if (sdcard_is_mounted) return ESP_OK; //Already mounted
	
	#ifdef CONFIG_DRIVER_SDCARD_MPR121_PIN
		driver_mpr121_set_gpio_level(CONFIG_DRIVER_SDCARD_MPR121_PIN, true); //Enable power
	#endif
	
	#ifdef CONFIG_DRIVER_SDCARD_MODE_SPI
		sdmmc_host_t host = SDSPI_HOST_DEFAULT();
		
		#ifdef CONFIG_DRIVER_SDCARD_BUS_HSPI
			host.slot = HSPI_HOST;
		#else
			host.slot = VSPI_HOST;
		#endif
		
		sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
		slot_config.gpio_miso   = CONFIG_DRIVER_SDCARD_PIN_MISO;
		slot_config.gpio_mosi   = CONFIG_DRIVER_SDCARD_PIN_MOSI;
		slot_config.gpio_sck    = CONFIG_DRIVER_SDCARD_PIN_CLK;
		slot_config.gpio_cs     = CONFIG_DRIVER_SDCARD_PIN_CS;
		slot_config.dma_channel = CONFIG_DRIVER_SDCARD_DMA_CHANNEL;
		
		gpio_pad_select_gpio(CONFIG_DRIVER_SDCARD_PIN_MISO);
		gpio_pad_select_gpio(CONFIG_DRIVER_SDCARD_PIN_MOSI);
		gpio_pad_select_gpio(CONFIG_DRIVER_SDCARD_PIN_CLK);
		gpio_pad_select_gpio(CONFIG_DRIVER_SDCARD_PIN_CS);
		
		gpio_set_direction(CONFIG_DRIVER_SDCARD_PIN_MISO, GPIO_MODE_INPUT_OUTPUT_OD);
		gpio_set_direction(CONFIG_DRIVER_SDCARD_PIN_MOSI, GPIO_MODE_INPUT_OUTPUT_OD);
		gpio_set_direction(CONFIG_DRIVER_SDCARD_PIN_CLK,  GPIO_MODE_INPUT_OUTPUT_OD);
		gpio_set_direction(CONFIG_DRIVER_SDCARD_PIN_CS,   GPIO_MODE_INPUT_OUTPUT);
		
		gpio_set_pull_mode(CONFIG_DRIVER_SDCARD_PIN_MISO, GPIO_PULLUP_ONLY);
		gpio_set_pull_mode(CONFIG_DRIVER_SDCARD_PIN_MOSI, GPIO_PULLUP_ONLY);
		gpio_set_pull_mode(CONFIG_DRIVER_SDCARD_PIN_CLK,  GPIO_PULLUP_ONLY);
		gpio_set_pull_mode(CONFIG_DRIVER_SDCARD_PIN_CS,   GPIO_PULLUP_ONLY);
		
		gpio_set_level(CONFIG_DRIVER_SDCARD_PIN_MISO, 1);
		gpio_set_level(CONFIG_DRIVER_SDCARD_PIN_MOSI, 1);
		gpio_set_level(CONFIG_DRIVER_SDCARD_PIN_CLK,  1);
		gpio_set_level(CONFIG_DRIVER_SDCARD_PIN_CS,   1);
		
	#else //1-line or 4-line SD mode
		sdmmc_host_t host = SDMMC_HOST_DEFAULT();
		sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
		
		#ifdef CONFIG_DRIVER_SDCARD_MODE_1LINE
			host.flags = SDMMC_HOST_FLAG_1BIT;
			slot_config.width = 1;
		#endif
		
		gpio_pad_select_gpio(SDMMC_HOST_PIN_CMD);
		gpio_set_direction(SDMMC_HOST_PIN_CMD, GPIO_MODE_INPUT_OUTPUT_OD);
		gpio_set_pull_mode(SDMMC_HOST_PIN_CMD, GPIO_PULLUP_ONLY);
		gpio_set_level(SDMMC_HOST_PIN_CMD, 1);
		
		gpio_pad_select_gpio(SDMMC_HOST_PIN_CLK);
		gpio_set_direction(SDMMC_HOST_PIN_CLK, GPIO_MODE_INPUT_OUTPUT_OD);
		gpio_set_pull_mode(SDMMC_HOST_PIN_CLK, GPIO_PULLUP_ONLY);
		gpio_set_level(SDMMC_HOST_PIN_CLK, 1);
		
		gpio_pad_select_gpio(SDMMC_HOST_PIN_D0);
		gpio_set_direction(SDMMC_HOST_PIN_D0,  GPIO_MODE_INPUT_OUTPUT_OD);
		gpio_set_pull_mode(SDMMC_HOST_PIN_D0,  GPIO_PULLUP_ONLY);
		gpio_set_level(SDMMC_HOST_PIN_D0,  1);
		
		#ifdef CONFIG_DRIVER_SDCARD_MODE_4LINE
			host.flags = SDMMC_HOST_FLAG_4BIT;
			slot_config.width = 4;
			
			gpio_pad_select_gpio(SDMMC_HOST_PIN_D1);
			gpio_set_direction(SDMMC_HOST_PIN_D1, GPIO_MODE_INPUT_OUTPUT_OD);
			gpio_set_pull_mode(SDMMC_HOST_PIN_D1, GPIO_PULLUP_ONLY);
			gpio_set_level(SDMMC_HOST_PIN_D1, 1);
			
			gpio_pad_select_gpio(SDMMC_HOST_PIN_D2);
			gpio_set_direction(SDMMC_HOST_PIN_D2, GPIO_MODE_INPUT_OUTPUT_OD);
			gpio_set_pull_mode(SDMMC_HOST_PIN_D2, GPIO_PULLUP_ONLY);
			gpio_set_level(SDMMC_HOST_PIN_D2, 1);
			
			gpio_pad_select_gpio(SDMMC_HOST_PIN_D3);
			gpio_set_direction(SDMMC_HOST_PIN_D3, GPIO_MODE_INPUT_OUTPUT_OD);
			gpio_set_pull_mode(SDMMC_HOST_PIN_D3, GPIO_PULLUP_ONLY);
			gpio_set_level(SDMMC_HOST_PIN_D3, 1);
		#endif
	#endif
	
	esp_vfs_fat_sdmmc_mount_config_t mount_config = {
		.format_if_mount_failed = format_if_mount_failed,
		.max_files              = CONFIG_DRIVER_SDCARD_FATFS_MAX_FILES,
		.allocation_unit_size   = 0
	};
	
	sdmmc_card_t* card;
	esp_err_t res = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
	
	if (res != ESP_OK) {
		if (res == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount the SD card filesystem.");
		} else if (res == ESP_ERR_NO_MEM) {
			ESP_LOGE(TAG, "Failed to initialize the SD card: not enough memory).");
		} else if (res == ESP_ERR_INVALID_RESPONSE) {
			ESP_LOGE(TAG, "Failed to initialize the SD card: invalid response).");
		} else if (res == ESP_ERR_INVALID_STATE) {
			ESP_LOGE(TAG, "Failed to initialize the SD card: invalid state).");
		} else {
			ESP_LOGE(TAG, "Failed to initialize the SD card (%s). ", esp_err_to_name(res));
		}
		return res;
	}
	
	//sdmmc_card_print_info(stdout, card);
	
	sdcard_is_mounted = true;
	return ESP_OK;
}

esp_err_t driver_sdcard_init(void)
{
	static bool driver_sdcard_init_done = false;
	if (driver_sdcard_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	
	//driver_sdcard_mount("/_#!#_sdcard", false);
	
	driver_sdcard_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

#else // CONFIG_DRIVER_SDCARD_ENABLE
esp_err_t driver_sdcard_init(void) { return ESP_OK; } // Dummy function, leave empty!
#endif
