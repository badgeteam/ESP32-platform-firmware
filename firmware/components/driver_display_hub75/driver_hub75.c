#include <esp_err.h>
#include <sdkconfig.h>

#ifdef CONFIG_DRIVER_HUB75_ENABLE

#include "include/driver_hub75.h"
#include "include/displayDriver.h"

/***
*  The HUB75 driver needs the following settings in sdkconfig:
*
* CONFIG_MICROPY_USE_BOTH_CORES=
* CONFIG_INT_WDT_CHECK_CPU1=y
* CONFIG_TASK_WDT_CHECK_IDLE_TASK_CPU1=y
* CONFIG_ESP32_WIFI_TASK_PINNED_TO_CORE_0=y
* CONFIG_ESP32_WIFI_TASK_PINNED_TO_CORE_1=
* CONFIG_FREERTOS_UNICORE=
***/


/**
 * Will be called from /firmware/main/main.c during initialisation using the INIT_DRIVER macro.
 */
esp_err_t driver_hub75_init(void) {
    displayDriver_init();

    return ESP_OK;
}


#else // DRIVER_HUB75_ENABLE
esp_err_t driver_hub75_init(void) { return ESP_OK; } // Dummy function, leave empty!
#endif