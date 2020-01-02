#ifndef DRIVER_MICROPHONE_INTERNAL_H
#define DRIVER_MICROPHONE_INTERNAL_H

typedef struct {
  // Type of encoded data
  mic_encoding data_type;
  // Length of the data, in bytes
  uint32_t data_size;
  // The actual data
  void *data;
} mic_data;

void driver_microphone_ring_buffer_free();
void driver_microphone_ring_buffer_init(size_t size);

esp_err_t driver_microphone_ring_buffer_get(mic_data *data);
esp_err_t driver_microphone_ring_buffer_put(mic_encoding type, void *buffer, size_t size);

#endif
