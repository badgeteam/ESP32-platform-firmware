#include "include/nvs.h"
#include "include/platform.h"
#include "include/ota_update.h"

extern void micropython_entry(void);

void app_main()
{
	logo();
	bool is_first_boot = nvs_init();
	platform_init();
	
	if (is_first_boot) { //Deze flag gebruiken we op het moment nergens meer voor.
		printf("\r\n\r\nAll your base are belong to us.\r\n\r\n");
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
