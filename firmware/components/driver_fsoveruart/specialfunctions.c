#include "include/packetutils.h"
#include "include/specialfunctions.h"
#include "include/driver_rtcmem.h"
#include <esp_sleep.h>
#include <esp_err.h>
#include <esp_log.h>

#define TAG "fsoveruart_sf"

int execfile(uint8_t *data, uint16_t command, uint32_t message_id, uint32_t size, uint32_t received, uint32_t length) {
    if(received != size) return 0;

    // char* filename = (char *) (data+1);   //Take length of the folder and add the spiflash mountpoint
    // while(*filename != '/') {
    //     filename++;
    // }
    ESP_LOGI(TAG, "Starting: %s", data);
    sendok(command, message_id);
    driver_rtcmem_string_write((char*) data);
    esp_deep_sleep(1000000);
    return 1;
}

int heartbeat(uint8_t *data, uint16_t command, uint32_t message_id, uint32_t size, uint32_t received, uint32_t length) {
    if(received != size) return 0;
    sendok(command, message_id);
    return 1;
}

int pythonstdin(uint8_t *data, uint16_t command, uint32_t message_id, uint32_t size, uint32_t received, uint32_t length) {
    FILE *fd;
    fd = fopen("/dev/fsou/2", "w");
    fwrite(data, 1, length, fd);
    fclose(fd);
    
    if(received != size) return 1;
    sendok(command, message_id);
    return 1;
}