#include <sdkconfig.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "sndmixer.h"
#include "libhelix-mp3/mp3dec.h"

#ifdef CONFIG_DRIVER_SNDMIXER_ENABLE

typedef struct {
	HMP3Decoder hMP3Decoder;

	// Input buffering
	uint8_t buff[1600]; // File buffer required to store at least a whole compressed frame
	int16_t buffValid;
	int16_t lastFrameEnd;
	bool FillBufferWithValidFrame(); // Read until we get a valid syncword and min(feof, 2048) butes in the buffer

	// Output buffering
	int16_t outSample[1152 * 2]; // Interleaved L/R
	int16_t validSamples;
	int16_t curSample;

	// Each frame may change this if they're very strange, I guess
	unsigned int lastRate;
	int lastChannels;
} mp3_ctx_t;

int mp3_init_source(const void *data_start, const void *data_end, int req_sample_rate, void **ctx) {
	mp3_ctx_t *mp3=calloc(sizeof(mp3_ctx_t), 1);
	if (!mp3) return -1;
	mp3->hMP3Decoder = MP3InitDecoder();
	if (!mp3->hMP3Decoder) {
		printf("Out of memory error! hMP3Decoder is NULL\n");
		return -1;
	}
	
}

int mp3_get_sample_rate(void *ctx) {
	return 0;
}

int mp3_fill_buffer(void *ctx, int8_t *buffer) {
	return 0;
}

void mp3_deinit_source(void *ctx) {
	
}

const sndmixer_source_t sndmixer_source_mp3={
	.init_source=mp3_init_source,
	.get_sample_rate=mp3_get_sample_rate,
	.fill_buffer=mp3_fill_buffer,
	.deinit_source=mp3_deinit_source
};

#endif
