#include <esp_err.h>

#include "include/driver_hub75.h"
#include "include/displayDriver.h"


/**
 * Will be called from /firmware/main/main.c during initialisation (be sure to #include <driver_hub75.h> there).
 */
esp_err_t driver_hub75_init(void) {

#ifdef CONFIG_DRIVER_I2C_ENABLE
    displayDriver_init();
#endif

    return ESP_OK;
}