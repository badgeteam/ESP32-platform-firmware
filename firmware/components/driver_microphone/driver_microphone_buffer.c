#include <stdlib.h>
#include <string.h>

#include "esp_err.h"

#include "include/driver_microphone.h"
#include "include/driver_microphone_internal.h"

static mic_data *g_data       = NULL;
volatile static size_t g_size = 0;
volatile static size_t g_head = 0;
volatile static size_t g_tail = 0;

static inline size_t get_used() {
  size_t tail = g_tail;
  size_t head = g_head;
  size_t size = g_size;

  if (head < tail) {
    return size - (tail - head) + 1;
  } else {
    return head - tail;
  }
}

static inline size_t get_free() {
  size_t tail = g_tail;
  size_t head = g_head;
  size_t size = g_size;
  if (head < tail) {
    return tail - head - 1;
  } else {
    return size - (head - tail);
  }
}

esp_err_t driver_microphone_ring_buffer_put(mic_encoding type, void *buffer, size_t size) {
  if (get_free() > 0) {
    size_t head            = g_head;
    g_data[head].data_type = type;
    g_data[head].data_size = size;
    g_data[head].data      = malloc(size);
    memcpy(g_data[head].data, buffer, size);
    g_head = (head + 1) % (g_size + 1);
    return ESP_OK;
  }
  return ESP_ERR_INVALID_STATE;
}

esp_err_t driver_microphone_ring_buffer_get(mic_data *data) {
  if (get_used()) {
    size_t index       = g_tail;
    *data              = g_data[index];
    g_data[index].data = NULL;
    g_tail             = (index + 1) % (g_size + 1);
    return ESP_OK;
  }
  return ESP_ERR_INVALID_STATE;
}

void driver_microphone_ring_buffer_free() {
  mic_data data;
  if (g_data) {
    while (driver_microphone_ring_buffer_get(&data) == ESP_OK) {
      free(data.data);
    }
    free(g_data);
    g_data = NULL;
  }
  g_size = 0;
  g_head = 0;
  g_tail = 0;
}

void driver_microphone_ring_buffer_init(size_t size) {
  driver_microphone_ring_buffer_free();
  g_data = calloc(size + 1, sizeof(mic_data));
  g_size = size;
  g_head = g_tail = 0;
}
