#include "include/system.h"

#include "driver_rtcmem.h"

extern int esp_rtcmem_read(uint32_t location);

void restart()
{
	for (int i = 10; i >= 0; i--) {
		printf("Restarting in %d seconds...\n", i);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	printf("Restarting now.\n");
	fflush(stdout);
	esp_restart();
}

void halt()
{
	printf("--- HALTED ---\n");
	fflush(stdout);
	while (1) { esp_deep_sleep_start(); }
}

