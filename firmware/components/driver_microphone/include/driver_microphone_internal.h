#ifndef DRIVER_MICROPHONE_INTERNAL_H
#define DRIVER_MICROPHONE_INTERNAL_H

void driver_microphone_ring_buffer_free();
void driver_microphone_ring_buffer_init(size_t size);

esp_err_t driver_microphone_ring_buffer_put(void *buffer, size_t size);

#endif
