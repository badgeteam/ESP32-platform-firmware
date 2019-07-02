#include "include/nvs.h"
#include "include/platform.h"
#include "include/ota_update.h"

extern void micropython_entry(void);

void app_main()
{
	logo();
	bool is_first_boot = nvs_init();
	platform_init();
	int magic = get_magic();
	if (is_first_boot) magic = MAGIC_FIRST_BOOT; //Override boot mode to first boot after formatting NVS.
	
	switch(magic) {
		case MAGIC_OTA:
			badge_ota_update();
			break;
		case MAGIC_FIRST_BOOT:
			platform_first_boot();
			break;
		default:
			micropython_entry();
	}
}
