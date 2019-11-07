#include "include/nvs.h"
#include "include/platform.h"
#include "include/ota_update.h"
#include "driver_framebuffer.h"

extern void micropython_entry(void);

extern esp_err_t unpack_first_boot_zip(void);

void app_main()
{
	logo();
	bool is_first_boot = nvs_init();
	platform_init();

	if (is_first_boot) {
		driver_framebuffer_fill(NULL, COLOR_BLACK);
		driver_framebuffer_print(NULL, "Extracting ZIP...\n", 0, 0, 1, 1, COLOR_WHITE, &roboto12pt7b);
		driver_framebuffer_flush(0);
		printf("Attempting to unpack FAT initialization ZIP file...\b");
		if (unpack_first_boot_zip() != ESP_OK) { //Error
			driver_framebuffer_fill(NULL, COLOR_BLACK);
			driver_framebuffer_print(NULL, "ZIP error\nRESET TO SKIP\n", 0, 0, 1, 1, COLOR_WHITE, &roboto12pt7b);
			driver_framebuffer_flush(0);
			printf("An error occured while unpacking the ZIP file!\b");
		} else { //Done
			esp_restart();
		}
	} else {
		int magic = get_magic();
		
		switch(magic) {
			case MAGIC_OTA:
				badge_ota_update();
				break;
			default:
				micropython_entry();
		}
	}
}
