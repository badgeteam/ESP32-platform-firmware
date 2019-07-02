#include "include/nvs.h"
#include "include/platform.h"
#include "include/ota_update.h"

extern void micropython_entry(void);

void app_main()
{
	logo();
	nvs_init();
	platform_init();
	int magic = get_magic();
	switch(magic) {
		case 1:
			badge_ota_update();
			break;
		default:
			micropython_entry();
	}
}
