#ifndef I2S_PARALLEL_H
#define I2S_PARALLEL_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    I2S_PARALLEL_BITS_8=8,
    I2S_PARALLEL_BITS_16=16,
    I2S_PARALLEL_BITS_32=32,
} i2s_parallel_cfg_bits_t;

typedef struct i2s_parallel_buffer_desc {
    void *memory;
    size_t size;
} i2s_parallel_buffer_desc_t;

void i2sparallel_init(i2s_parallel_buffer_desc_t *bufa, i2s_parallel_buffer_desc_t *bufb);
void i2sparallel_flipBuffer(int bufid);

#endif
