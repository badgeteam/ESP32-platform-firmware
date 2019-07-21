#include <sdkconfig.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <esp_err.h>

#include "include/driver_vspi.h"

#ifdef CONFIG_DRIVER_VSPI_ENABLE

#define TAG "vspi"

esp_err_t (*driver_vspi_release)(void) = NULL;

/** initialize spi sharing
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_vspi_init(void)
{
	static bool driver_vspi_init_done = false;
	if (driver_vspi_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");
	// TODO: create mutex
	driver_vspi_init_done = true;
	ESP_LOGD(TAG, "init done");
	return ESP_OK;
}

/** force other device to release the vspi
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_vspi_release_and_claim(esp_err_t (*release)(void))
{
	ESP_LOGD(TAG, "release and claim");
	// TODO: grab lock

	esp_err_t res;
	if (driver_vspi_release == NULL) {
		driver_vspi_release = release;
		res = ESP_OK;

	} else {
		res = driver_vspi_release();
		if (driver_vspi_release == NULL && res == ESP_OK) {
			driver_vspi_release = release;

		} else if (res == ESP_OK) {
			res = ESP_FAIL;
		}
	}

	// TODO: release lock

	return res;
}

extern esp_err_t driver_vspi_freed(void)
{
	ESP_LOGD(TAG, "freed");
	// TODO: check if mutex is claimed

	driver_vspi_release = NULL;

	return ESP_OK;
}

#else // CONFIG_DRIVER_VSPI_ENABLE
esp_err_t driver_vspi_init(void) { return ESP_OK; } // Dummy function, leave empty!
#endif
