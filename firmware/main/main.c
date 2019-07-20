#include "include/nvs.h"
#include "include/platform.h"
#include "include/ota_update.h"

extern void micropython_entry(void);

extern esp_err_t unpack_first_boot_zip(void);

void app_main()
{
	logo();
	bool is_first_boot = nvs_init();
	platform_init();
	
	if (is_first_boot) {
		printf("Executing first boot procedure...\b");
		if (unpack_first_boot_zip() != ESP_OK) {
			printf("First boot failed, HALT!\b");
			halt();
		} else {
			printf("First boot completed succesfully, RESTART!\b");
			fflush(stdout);
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
