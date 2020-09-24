#ifndef DRIVER_MICROPHONE_H
#define DRIVER_MICROPHONE_H

#include <stdint.h>
#include <esp_err.h>

typedef enum {
  MIC_SAMP_RATE_8_KHZ,
  MIC_SAMP_RATE_12_KHZ,
  MIC_SAMP_RATE_16_KHZ,
  MIC_SAMP_RATE_24_KHZ,
  MIC_SAMP_RATE_48_KHZ
} mic_sampling_rate;

typedef struct {
  // Length of the data, in bytes
  uint32_t data_size;
  // The actual data
  void *data;
} mic_data;

__BEGIN_DECLS
extern esp_err_t driver_microphone_init();
extern int driver_microphone_running();
extern uint32_t driver_microphone_get_sampling_rate();
extern esp_err_t driver_microphone_start(mic_sampling_rate rate, uint16_t frame_size,
                                         uint8_t frame_backlog);
esp_err_t driver_microphone_ring_buffer_get(mic_data *data);
size_t driver_microphone_ring_buffer_peek_size();
extern void driver_microphone_stop();
__END_DECLS

#endif  // BADGE_MICROPHONE_H
