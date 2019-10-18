//Author: Renze Nicolai 2019, badge.team

#include <sdkconfig.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "sndmixer.h"

#ifdef CONFIG_DRIVER_SNDMIXER_ENABLE

#define CHUNK_SIZE 32

typedef struct {
	int sampleRate;
	int pos;
	uint16_t frequency;
} synth_ctx_t;

int synth_init_source(const void *data_start, const void *data_end, int req_sample_rate, void **ctx) {
	synth_ctx_t *synth=calloc(sizeof(synth_ctx_t), 1);
	if (!synth) return -1;
	
	synth->sampleRate = 22050;//req_sample_rate;	
	synth->frequency  = 1000;
	
	*ctx = (void*) synth;
	
	printf("SYNTH created, requested sample rate: %d\n", req_sample_rate);

	return CHUNK_SIZE; //Chunk size
}

int synth_get_sample_rate(void *ctx) {
	synth_ctx_t *synth = (synth_ctx_t*) ctx;
	printf("synth_get_sample_rate called @ %p : %d\n", ctx, synth->sampleRate);
	return synth->sampleRate;
}

int synth_fill_buffer(void *ctx, int8_t *buffer) {
	synth_ctx_t *synth = (synth_ctx_t*) ctx;
	
	if (synth->frequency == 0) {
		for (int i = 0; i < CHUNK_SIZE; i++) buffer[i] = 0;
		synth->pos = 0;
		return CHUNK_SIZE;
	}
	
	int samplesPerWavelength = synth->sampleRate / synth->frequency;
	
	for (int i = 0; i < CHUNK_SIZE; i++) {
		synth->pos += 1;
		if (synth->pos >= samplesPerWavelength) synth->pos = 0;
		int8_t value = -128 + ((synth->pos*256)/samplesPerWavelength);
		buffer[i] = value;
		//printf("SYNTH %u of %u: %d\n", synth->pos, samplesPerWavelength, value);
	}
	return CHUNK_SIZE;
}

void synth_deinit_source(void *ctx) {
	synth_ctx_t *synth = (synth_ctx_t*) ctx;
	free(synth);
}

void synth_set_frequency(void *ctx, uint16_t frequency) {
	synth_ctx_t *synth = (synth_ctx_t*) ctx;
	synth->frequency = frequency;
}

const sndmixer_source_t sndmixer_source_synth={
	.init_source=synth_init_source,
	.get_sample_rate=synth_get_sample_rate,
	.fill_buffer=synth_fill_buffer,
	.deinit_source=synth_deinit_source,
	.set_frequency=synth_set_frequency
};

#endif
