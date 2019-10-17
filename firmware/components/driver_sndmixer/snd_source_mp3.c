//Author: Renze Nicolai 2019, badge.team

#include <sdkconfig.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "sndmixer.h"
#include "libhelix-mp3/mp3dec.h"

#ifdef CONFIG_DRIVER_SNDMIXER_ENABLE

#define CHUNK_SIZE 32

typedef struct {
	HMP3Decoder hMP3Decoder;
	unsigned char *dataStart;
	unsigned char *dataCurr;
	unsigned char *dataEnd;
	int lastRate;
	int lastChannels;
	short* buffer;
	int bufferValid;
	int bufferOffset;
} mp3_ctx_t;

int mp3_decode(void *ctx)
{
	mp3_ctx_t *mp3 = (mp3_ctx_t*) ctx;
	int available = mp3->dataEnd - mp3->dataCurr + 1;
	int nextSync = MP3FindSyncWord(mp3->dataCurr, available);
	if (nextSync >= 0) {
		mp3->dataCurr += nextSync;
		available = mp3->dataEnd - mp3->dataCurr + 1;
		
		//printf("Next syncword @ %d, available = %d\n", nextSync, available);
		int ret = MP3Decode(mp3->hMP3Decoder, &mp3->dataCurr, &available, mp3->buffer, 0);
				
		if (ret) {
			printf("MP3Decode error %d\n", ret);
			return 0;
		}
		
		MP3FrameInfo fi;
		MP3GetLastFrameInfo(mp3->hMP3Decoder, &fi);
		mp3->lastRate     = fi.samprate;
		mp3->lastChannels = fi.nChans;
		int validSamples = fi.outputSamps / mp3->lastChannels;
		//printf("MP3Decode OK, buffer @ %p, available = %d, rate = %d, channels = %d, validSamples = %d\n", mp3->dataCurr, available, mp3->lastRate, mp3->lastChannels, validSamples);
		
		mp3->bufferValid  = validSamples;
		mp3->bufferOffset = 0;
		return 1;
	} else {
		//printf("No syncword found\n");
		return 0;
	}
}

int mp3_init_source(const void *data_start, const void *data_end, int req_sample_rate, void **ctx) {
	//Allocate space for the information struct
	mp3_ctx_t *mp3=calloc(sizeof(mp3_ctx_t), 1);
	if (!mp3) return -1;
	
	//Start the MP3 library
	mp3->hMP3Decoder = MP3InitDecoder();
	if (!mp3->hMP3Decoder) {
		printf("Out of memory error! hMP3Decoder is NULL\n");
		return -1;
	}
	
	//Fill the struct with info
	mp3->dataStart    = (unsigned char*) data_start; //Start of data
	mp3->dataCurr     = (unsigned char*) data_start; //Current position
	mp3->dataEnd      = (unsigned char*) data_end; //End of data
	mp3->lastRate     = 0;
	mp3->lastChannels = 0;
	mp3->buffer       = calloc(1152 * 2, sizeof(short));
	mp3->bufferValid  = 0;
	mp3->bufferOffset = 0;
	
	uint32_t length = data_end - data_start + 1;
	
	printf("MP3 source started, data at %p with size %u!\n", mp3->dataStart, length);
		
	*ctx = (void*) mp3;
	
	mp3_decode((void*) mp3); //Decode first part
	
	return CHUNK_SIZE; //Chunk size
}

int mp3_get_sample_rate(void *ctx) {
	mp3_ctx_t *mp3 = (mp3_ctx_t*) ctx;
	printf("mp3_get_sample_rate called @ %p : %d\n", ctx, mp3->lastRate);
	return mp3->lastRate;
}

int mp3_fill_buffer(void *ctx, int8_t *buffer) {
	mp3_ctx_t *mp3 = (mp3_ctx_t*) ctx;
	if (mp3->bufferValid < 1) mp3_decode(ctx);	
	if (mp3->bufferValid > 0) {
		int len = mp3->bufferValid;
		if (len > CHUNK_SIZE) len = CHUNK_SIZE;
		//printf("mp3_fill_buffer sending %d of %d, offset = %d\n", len, mp3->bufferValid, mp3->bufferOffset);
		//memcpy(buffer, &mp3->buffer[mp3->bufferOffset], len); //Won't work due to size difference
		
		for (int i=0; i<len; i++) {
			buffer[i] = mp3->buffer[mp3->bufferOffset + i] >> 8; //MP3 gives 16-bit audio, make that 8 :D
		}
		
		mp3->bufferValid -= len;
		mp3->bufferOffset += len;
		return len;
	}
	
	return 0;
}

void mp3_deinit_source(void *ctx) {
	mp3_ctx_t *mp3 = (mp3_ctx_t*) ctx;
	MP3FreeDecoder(mp3->hMP3Decoder);
	free(mp3->buffer);
	free(mp3);
}

const sndmixer_source_t sndmixer_source_mp3={
	.init_source=mp3_init_source,
	.get_sample_rate=mp3_get_sample_rate,
	.fill_buffer=mp3_fill_buffer,
	.deinit_source=mp3_deinit_source
};

#endif
