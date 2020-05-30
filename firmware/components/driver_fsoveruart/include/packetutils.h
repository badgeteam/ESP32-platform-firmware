#ifndef PACKETUTILS_H
#define PACKETUTILS_H

#include <stdint.h>

#define RD_BUF_SIZE 512

void createMessageHeader(uint8_t *header, uint16_t command, uint32_t size);
void sendok(uint16_t command);
void sender(uint16_t command);
void sendte(uint16_t command);
void sendto(uint16_t command);
void buildfile(char *source, char *target);

#endif