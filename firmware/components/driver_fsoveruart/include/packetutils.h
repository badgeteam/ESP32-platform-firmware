#ifndef PACKETUTILS_H
#define PACKETUTILS_H

#include <stdint.h>


void createMessageHeader(uint8_t *header, uint16_t command, uint32_t size);
void sendok(uint16_t command);
void sender(uint16_t command);

#endif