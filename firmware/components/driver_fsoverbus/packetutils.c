#include "include/packetutils.h"
#include "include/fsob_backend.h"
#include <string.h>
#include <esp_log.h>

#define TAG "fsoveruart_pu"

void createMessageHeader(uint8_t *header, uint16_t command, uint32_t size, uint32_t messageid) {
    //ESP_LOGI(TAG, "Reply: %d %d %d", command, size, messageid);
    uint16_t *com = (uint16_t *) header;
    *com = command;
    uint32_t *siz = (uint32_t *) &header[2];
    *siz = size;
    header[6] = 0xDE;
    header[7] = 0xAD;
    uint32_t *id = (uint32_t *) &header[8];
    *id = messageid;
}

//Error executing function
void sender(uint16_t command, uint32_t message_id) {
     uint8_t header[PACKET_HEADER_SIZE+3];
    createMessageHeader(header, command, 3, message_id);
    strcpy((char *) &header[PACKET_HEADER_SIZE], "er");
    fsob_write_bytes((const char*) header, 15);
}

//Okay
void sendok(uint16_t command, uint32_t message_id) {
     uint8_t header[PACKET_HEADER_SIZE+3];
    createMessageHeader(header, command, 3, message_id);
    strcpy((char *) &header[PACKET_HEADER_SIZE], "ok");
    fsob_write_bytes((const char*) header, 15);
}

//Transmission error
void sendte(uint16_t command, uint32_t message_id) {
     uint8_t header[PACKET_HEADER_SIZE+3];
    createMessageHeader(header, command, 3, message_id);
    strcpy((char *) &header[PACKET_HEADER_SIZE], "te");
    fsob_write_bytes((const char*) header, 15);
}

//Timeout error
void sendto(uint16_t command, uint32_t message_id) {
     uint8_t header[PACKET_HEADER_SIZE+3];
    createMessageHeader(header, command, 3, message_id);
    strcpy((char *) &header[PACKET_HEADER_SIZE], "to");
    fsob_write_bytes((const char*) header, 15);
}

void buildfile(char *source, char *target) {
    if(strncmp(source, "/flash", 6) == 0) {
        strcpy(target, "/_#!#_spiflash");
        strcat(target, &source[6]);
    } else if(strncmp(source, "/sdcard", 7) == 0) {
        strcpy(target, "/_#!#_sdcard");
        strcat(target, &source[7]);
    }
}