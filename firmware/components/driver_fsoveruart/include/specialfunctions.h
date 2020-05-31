#ifndef SPECIALFUNCTIONS_H
#define SPECIALFUNCTIONS_H

#include <stdint.h>
#include <esp_err.h>

int execfile(uint8_t *data, uint16_t command, uint32_t message_id, uint32_t size, uint32_t received, uint32_t length);
int heartbeat(uint8_t *data, uint16_t command, uint32_t message_id, uint32_t size, uint32_t received, uint32_t length);

#endif


