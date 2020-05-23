#include "include/packetutils.h"
#include "include/specialfunctions.h"
#include "include/driver_rtcmem.h"
#include <esp_sleep.h>
#include <esp_err.h>
#include <esp_log.h>

#define TAG "fsoveruart_sf"

int execfile(uint8_t *data, uint16_t command, uint32_t size, uint32_t received, uint32_t length) {
    if(received != size) return 0;

    char* filename = (char *) (data+1);   //Take length of the folder and add the spiflash mountpoint
    while(*filename != '/') {
        filename++;
    }
    ESP_LOGI(TAG, "Starting: %s", filename);
    sendok(command);
    driver_rtcmem_string_write(filename);
    esp_deep_sleep(1000000);
    return 1;
}