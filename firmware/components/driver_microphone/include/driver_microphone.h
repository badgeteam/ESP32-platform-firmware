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

typedef enum { MIC_ENCODING_PCM_8_BIT, MIC_ENCODING_PCM_16_BIT, MIC_ENCODING_OPUS } mic_encoding;

__BEGIN_DECLS

extern esp_err_t driver_microphone_init(mic_sampling_rate rate, mic_encoding encoding,
                                        uint16_t frame_size, uint8_t frame_backlog);
extern uint32_t driver_microphone_get_sampling_rate();

__END_DECLS

#endif  // BADGE_MICROPHONE_H
