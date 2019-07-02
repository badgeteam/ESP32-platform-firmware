#include <esp_err.h>
#include <sdkconfig.h>

#ifdef CONFIG_DRIVER_HUB75_ENABLE

#include "include/driver_hub75.h"
#include "include/displayDriver.h"


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