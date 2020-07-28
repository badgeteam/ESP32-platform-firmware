#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "esp_wifi.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "include/factory_reset.h"
#include "driver_framebuffer.h"
#include "driver_framebuffer_devices.h"

#include "compositor.h"

#include "esp_partition.h"
#include "esp_spi_flash.h"

#define TAG "factory-reset"

void graphics_show_fr(const char* text, uint8_t percentage, bool showPercentage, bool force)
{
		#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE
			#if defined(CONFIG_DRIVER_EINK_ENABLE) || defined(CONFIG_DRIVER_ILI9341_ENABLE) || defined(CONFIG_DRIVER_GXGDE0213B1_ENABLE)
				driver_framebuffer_fill(NULL, COLOR_WHITE);

				driver_framebuffer_print(NULL, "Erasing storage...", 0, 4,  1, 1, COLOR_BLACK, &permanentmarker_22pt7b);
				driver_framebuffer_print(NULL, text,            0, 40, 1, 1, COLOR_BLACK, &roboto_12pt7b         );
				if (showPercentage) {
					char buffer[16];
					snprintf(buffer, 16, "%*u%%", 3, percentage);
					driver_framebuffer_print(
						NULL,
						buffer,
						driver_framebuffer_getWidth(NULL)-driver_framebuffer_get_string_width(buffer, &permanentmarker_36pt7b),
						driver_framebuffer_getHeight(NULL)-driver_framebuffer_get_string_height(buffer, &permanentmarker_36pt7b),
						1,
						1,
						COLOR_BLACK,
						&permanentmarker_36pt7b
					);
				}
				driver_framebuffer_print(NULL, "BADGE.TEAM ESP32 PLATFORM", 0, driver_framebuffer_getHeight(NULL)-15, 1, 1, COLOR_BLACK, &fairlight_8pt7b);
				driver_framebuffer_flush(force ? FB_FLAG_FORCE+FB_FLAG_FULL+FB_FLAG_LUT_NORMAL : FB_FLAG_LUT_FAST);
			#endif
			#if defined(CONFIG_DRIVER_SSD1306_ENABLE) || defined(CONFIG_DRIVER_ERC12864_ENABLE)
				driver_framebuffer_fill(NULL, COLOR_FILL_DEFAULT);
				driver_framebuffer_print(NULL, "Erasing storage...", 0, 0, 1, 1, COLOR_TEXT_DEFAULT, &roboto_12pt7b);
				driver_framebuffer_print(NULL, text, 0, 14, 1, 1, COLOR_TEXT_DEFAULT, &roboto_12pt7b);
				char buffer[16];
				snprintf(buffer, 16, "%*u%%", 3, percentage);
				driver_framebuffer_print(NULL, buffer, 0,30, 1, 1, COLOR_FILL_DEFAULT, &roboto_12pt7b);
				uint16_t progressPosition = (percentage*(FB_WIDTH-17))/100;
				driver_framebuffer_rect(NULL, 5, FB_HEIGHT-15, FB_WIDTH-10, 10, false, COLOR_TEXT_DEFAULT); //Outline of progress bar
				driver_framebuffer_rect(NULL, 6, FB_HEIGHT-14, progressPosition, 8, true, COLOR_TEXT_DEFAULT); //Progress bar
				driver_framebuffer_flush(0);
			#endif
			#ifdef CONFIG_DRIVER_HUB75_ENABLE
				compositor_disable(); //Don't use the compositor if we have the framebuffer driver
				driver_framebuffer_fill(NULL, COLOR_BLACK);
				uint16_t progressPosition = (percentage*FB_WIDTH)/100;
				for (uint8_t i = 0; i < FB_WIDTH; i++) {
					driver_framebuffer_line(NULL, i, FB_HEIGHT-1, i, FB_HEIGHT-1, (i < progressPosition) ? 0x008800 : 0x880000);
				}
				if (!showPercentage) {
					driver_framebuffer_print(NULL, text, 0, 0, 1, 1, COLOR_WHITE, &ipane7x5);
				} else {
					char buff[4];
					sprintf(buff, "%d%%", percentage);
					driver_framebuffer_print(NULL, buff, 0, 0, 1, 1, COLOR_WHITE, &ipane7x5);
				}
				driver_framebuffer_flush(0);
			#endif
			#ifdef CONFIG_DRIVER_DISPLAY_I2C_ENABLE
				driver_framebuffer_fill(NULL, 0x252525);
				uint32_t progressPosition = (percentage*FB_WIDTH*FB_HEIGHT)/100;
                                for(uint32_t i = 0; i < progressPosition; i++) {
                                  uint32_t x = i % FB_WIDTH;
                                  uint32_t y = i / FB_WIDTH;
                                  driver_framebuffer_setPixel(NULL, x, y, 0xFF7F00);
                                }
				driver_framebuffer_flush(0);

			#endif
		#else
			#ifdef CONFIG_DRIVER_HUB75_ENABLE
				compositor_enable();
				char buff[4];
				sprintf(buff, "%d%%", percentage);
				Color col = {.value=0xffffffff};
				compositor_clear();
				compositor_addText(buff, col, 6, 1);
			#endif
		#endif
}

static void __attribute__((noreturn)) task_fatal_error(void)
{
	ESP_LOGE(TAG, "Exiting task due to fatal error...");
	graphics_show_fr("Failed!", 0, false, true);
	vTaskDelay(5000 / portTICK_PERIOD_MS);
	esp_restart();
	(void)vTaskDelete(NULL);
}

static void factory_reset_task(void *pvParameter)
{
	esp_err_t err;

	graphics_show_fr("Erase starts in 2 seconds!", 0, false, true);
	vTaskDelay(2000 / portTICK_PERIOD_MS);

	const char* label = NULL; // Our FAT partition has label "locfd", not setting this wipes all FAT partitions on the internal flash chip
	esp_partition_iterator_t partitions = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, label);
	if (partitions == NULL) {
		printf("No FAT partitions found\n");
		graphics_show_fr("No FAT partitions found!", 0, false, true);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
	while (partitions) {
			const esp_partition_t* partition = esp_partition_get(partitions);
			printf("Found FAT partition '%s' at address 0x%08x (%u bytes)\n",
				partition->label,
				partition->address,
				partition->size
			);
			char text[256];
			snprintf(text, 256, "Erasing...\n(%s)", partition->label);
			graphics_show_fr(text, 0, false, true);
			uint32_t first_sector = partition->address / SPI_FLASH_SEC_SIZE;
			uint32_t amount_of_sectors = partition->size / SPI_FLASH_SEC_SIZE;
			if (partition->address % SPI_FLASH_SEC_SIZE) {
				printf(" -> Error: partition start is not aligned to flash sector size\n");
				graphics_show_fr("Error 1", 0, false, true);
				vTaskDelay(1000 / portTICK_PERIOD_MS);
			} else if (partition->size % SPI_FLASH_SEC_SIZE) {
				printf(" -> Error: partition size is not aligned to flash sector size\n");
				graphics_show_fr("Error 2", 0, false, true);
				vTaskDelay(1000 / portTICK_PERIOD_MS);
			} else {
				uint8_t prevProgress = 0;
				for (uint32_t i = 0; i < amount_of_sectors; i++) {
					uint8_t progress = (i*100)/amount_of_sectors;
					if (progress != prevProgress) {
						prevProgress = progress;
						printf(" -> Erasing '%s'... (%u%%)\n", partition->label, progress);
						graphics_show_fr(text, progress, true, false);
						vTaskDelay(1 / portTICK_PERIOD_MS);
					}
					err = spi_flash_erase_sector(first_sector+i);
					if (err != ESP_OK) {
						printf("Erase error %u\n", err);
						graphics_show_fr("Erase failed!", 0, false, true);
					}
					taskYIELD();
				}
				printf("'%s' has been erased\n", partition->label);
				graphics_show_fr("Operation completed", 0, false, true);
			}
			partitions = esp_partition_next(partitions);
			taskYIELD();
	}

	graphics_show_fr("Done, restarting...", 100, false, true);
	vTaskDelay(2000 / portTICK_PERIOD_MS);

	/*err = esp_ota_set_boot_partition(part_update);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "esp_ota_set_boot_partition failed! err=0x%x", err);
		task_fatal_error();
	}*/
	ESP_LOGW(TAG, "Prepare to restart system!");
	esp_restart();
	return;
}

void factory_reset() {
	xTaskCreatePinnedToCore(&factory_reset_task, "factory_reset_task", 8192, NULL, 3, NULL, 0);
}

