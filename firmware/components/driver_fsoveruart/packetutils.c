#include "include/packetutils.h"
#include <string.h>
#include <driver/uart.h>

void createMessageHeader(uint8_t *header, uint16_t command, uint32_t size) {
    uint16_t *com = (uint16_t *) header;
    *com = command;
    uint32_t *siz = (uint32_t *) &header[2];
    *siz = size;
    header[6] = 0xDE;
    header[7] = 0xAD;
}

//Error executing function
void sender(uint16_t command) {
     uint8_t header[11];
    createMessageHeader(header, command, 3);
    strcpy((char *) &header[8], "er");
    uart_write_bytes(CONFIG_DRIVER_FSOVERUART_UART_NUM, (const char*) header, 11);
}

//Okay
void sendok(uint16_t command) {
     uint8_t header[11];
    createMessageHeader(header, command, 3);
    strcpy((char *) &header[8], "ok");
    uart_write_bytes(CONFIG_DRIVER_FSOVERUART_UART_NUM, (const char*) header, 11);
}

//Transmission error
void sendte(uint16_t command) {
     uint8_t header[11];
    createMessageHeader(header, command, 3);
    strcpy((char *) &header[8], "te");
    uart_write_bytes(CONFIG_DRIVER_FSOVERUART_UART_NUM, (const char*) header, 11);
}

//Timeout error
void sendto(uint16_t command) {
     uint8_t header[11];
    createMessageHeader(header, command, 3);
    strcpy((char *) &header[8], "to");
    uart_write_bytes(CONFIG_DRIVER_FSOVERUART_UART_NUM, (const char*) header, 11);
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