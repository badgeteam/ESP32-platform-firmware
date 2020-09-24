#include <sdkconfig.h>
#include <string.h>
#include <fcntl.h>
#include <nvs_flash.h>
#include <nvs.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_spi_flash.h"
#include "esp_system.h"

#include "system.h"

esp_err_t nvs_format();
bool nvs_check_empty();
bool nvs_init();
