#include "include/nvs_init.h"
#include "include/platform.h"
#include "include/ota_update.h"
#include "driver_framebuffer.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

extern void micropython_entry(void);

extern esp_err_t unpack_first_boot_zip(void);

void nvs_write_zip_status(bool status)
{
	nvs_handle my_handle;
	esp_err_t res = nvs_open("system", NVS_READWRITE, &my_handle);
	if (res != ESP_OK) {
		printf("NVS seems unusable! Please erase flash and try flashing again. (1)\n");
		halt();
	}
	res = nvs_set_u8(my_handle, "preseed", status);
	if (res != ESP_OK) {
		printf("NVS seems unusable! Please erase flash and try flashing again. (1)\n");
		halt();
	}
}

void app_main()
{
	logo();
	bool is_first_boot = nvs_init();
	platform_init();

	if (is_first_boot) {
		#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE
			driver_framebuffer_fill(NULL, COLOR_BLACK);
			driver_framebuffer_print(NULL, "Extracting ZIP...\n", 0, 0, 1, 1, COLOR_WHITE, &roboto_12pt7b);
			driver_framebuffer_flush(0);
		#endif
		printf("Attempting to unpack FAT initialization ZIP file...\b");
		if (unpack_first_boot_zip() != ESP_OK) { //Error
			#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE
				driver_framebuffer_fill(NULL, COLOR_BLACK);
				driver_framebuffer_print(NULL, "ZIP error!\n", 0, 0, 1, 1, COLOR_WHITE, &roboto_12pt7b);
				driver_framebuffer_flush(0);
			#endif
			printf("An error occured while unpacking the ZIP file!");
			nvs_write_zip_status(false);
		} else {
			nvs_write_zip_status(true);
		}
		esp_restart();
	}
	
	int magic = get_magic();
	
	switch(magic) {
		case MAGIC_OTA:
			badge_ota_update();
			break;
		default:
			micropython_entry();
	}
}
