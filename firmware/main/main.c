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
	
	uint8_t passed_selftest = 0;
	esp_err_t res = config_get_u8("system", "selftest", &passed_selftest);
	if (res != ESP_OK) {
		printf("Flash might be corrupted, reformatting NVS... (%d)\n", res);
		nvs_format();
		restart();
	}
	
	if (is_first_boot || (!passed_selftest)) magic = MAGIC_FIRST_BOOT;
	
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
