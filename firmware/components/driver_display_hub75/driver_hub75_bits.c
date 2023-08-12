/* TEST:
 * gcc -o driver_hub75_bits driver_hub75_bits.c val2pwm.c -Wall -DDRIVER_HUB75_DMA_DATA_TEST -g
 *
 */

#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "include/val2pwm.h"
#include "include/driver_hub75_bits.h"
#include "include/i2s_parallel.h"


#ifdef DRIVER_HUB75_DMA_DATA_TEST
#define CONFIG_DRIVER_HUB75_ENABLE
#define CONFIG_HUB75_WIDTH 32
#define MALLOC_CAP_DMA (-1)
#include <stdlib.h>
#include <stdio.h>
void *heap_caps_calloc(size_t nmemb, size_t size, int cap) { return calloc(nmemb, size); }
void i2sparallel_init(i2s_parallel_buffer_desc_t *bufa, i2s_parallel_buffer_desc_t *bufb) { }
void i2sparallel_flipBuffer(int bufid) { }
#else
#include "esp_heap_caps.h"
#endif

#ifdef CONFIG_DRIVER_HUB75_ENABLE

#define BIT_RED   (1<<0)   //connected to GPIO2 here
#define BIT_GREEN (1<<1)   //connected to GPIO15 here
#define BIT_BLUE  (1<<2)   //connected to GPIO4 here
#define MASK_RGB (BIT_RED|BIT_GREEN|BIT_BLUE)

#define BIT_A (1<<3)    //connected to GPIO5 here
#define BIT_B (1<<4)    //connected to GPIO18 here
#define BIT_C (1<<5)   //connected to GPIO19 here

#define BIT_LATCH (1<<6) //connected to GPIO26 here
#define BIT_NOT_OUTPUT_ENABLE (1<<7)  //connected to GPIO25 here

#define N_ROWS (8)
#define N_COLUMNS (32)

#define BIT_DEPTH (12)

// data is written out as upside-down middle-endian
#define DMA_ORDER(x) ((x)^2)

/* BCM* modulation */

/*
	bits15_to_12, bit11, bit4_1x1, * for each row *
	bits15_to_12, bit10, bit6_1x4, * ...          *
	bits15_to_12, bit11, on_1x32   *              *
	bits15_to_12, bit9_1x32, bit7_1x8,

	bits15_to_12, bit11, on_1x32, 
	bits15_to_12, bit10, bit5_1x2,
	bits15_to_12, bit11, on_1x32,
	bits15_to_12, bit8_1x16, 
*/

/* BCM dithering data pointed to by DMS descriptors
 * bitXX[W] fields contain screen data so they need to be unique
 * try to pack them such that we don't need many dma descriptors
 * off_prev_row[W] contains address line data for the previously displayed row
 * in order to prevent ghosting.
 * off_prev_row & bit15 have not output enable
 */

#define ON_TIME_SLOT (N_COLUMNS)         /* must be multiple of 4 */
#define OFF_PREV_TIME_SLOT (1*N_COLUMNS)        /* must be multiple of 4 */
#define OFF_CUR_TIME_SLOT (1*N_COLUMNS)        /* must be multiple of 4 */
#define BITBANG_SLOT (N_COLUMNS)         /* must be multiple of 4 */
#define BITBANG_ON_TIME (BITBANG_SLOT-4)
#define PADD4(x) ( (x+3) & ~3 )

#define ON_TIME_BIT15  (8*ON_TIME_SLOT-BITBANG_ON_TIME)
#define ON_TIME_BIT14  (4*ON_TIME_SLOT-BITBANG_ON_TIME)
#define ON_TIME_BIT13  (2*ON_TIME_SLOT-BITBANG_ON_TIME)
#define ON_TIME_BIT12  (1*ON_TIME_SLOT-BITBANG_ON_TIME)
#define ON_TIME_BIT11  (1*ON_TIME_SLOT)
#define ON_TIME_BIT11B (1*ON_TIME_SLOT-BITBANG_ON_TIME)
#define ON_TIME_BIT10  (1*ON_TIME_SLOT-BITBANG_ON_TIME)
#define ON_TIME_BIT9   (1*ON_TIME_SLOT-BITBANG_ON_TIME)
#define ON_TIME_BIT8   (ON_TIME_SLOT/2)
#define ON_TIME_BIT7   (ON_TIME_SLOT/4)
#define ON_TIME_BIT6   (ON_TIME_SLOT/8)
#define ON_TIME_BIT5   (ON_TIME_SLOT/16)
#define ON_TIME_BIT4   (ON_TIME_SLOT/32)

typedef struct __attribute__((packed))
{
	/* common part: */
	uint8_t off_prev_row[PADD4(OFF_PREV_TIME_SLOT)];
	uint8_t off_cur_row[PADD4(OFF_CUR_TIME_SLOT)];
	uint8_t bit15[BITBANG_SLOT];
	uint8_t on_time_bit15[PADD4(ON_TIME_BIT15)];
	uint8_t bit14[BITBANG_SLOT];
	uint8_t on_time_bit14[PADD4(ON_TIME_BIT14)];
	uint8_t bit13[BITBANG_SLOT];
	uint8_t on_time_bit13[PADD4(ON_TIME_BIT13)];
	uint8_t bit12[BITBANG_SLOT];
	uint8_t on_time_bit12[PADD4(ON_TIME_BIT12)];
	/* line 0 & 2 & 4 & 6 */
	uint8_t bit11[BITBANG_SLOT];
	/* line 2 & 4 & 6 */
	uint8_t on_time_bit11[PADD4(ON_TIME_BIT11)];
	/* line 0 */
	uint8_t on_time_bit11b[PADD4(ON_TIME_BIT11B)];
	uint8_t bit4[BITBANG_SLOT];
	uint8_t on_time_bit4[PADD4(ON_TIME_BIT4)];
	/* line 1 & 5 */
	uint8_t bit10[BITBANG_SLOT];
	uint8_t on_time_bit10[PADD4(ON_TIME_BIT10)];
	/* line 1 */
	uint8_t bit6[BITBANG_SLOT];
	uint8_t on_time_bit6[PADD4(ON_TIME_BIT6)];
	/* line 3 */
	uint8_t bit9[BITBANG_SLOT];
	uint8_t on_time_bit9[PADD4(ON_TIME_BIT9)];
	uint8_t bit7[BITBANG_SLOT];
	uint8_t on_time_bit7[PADD4(ON_TIME_BIT7)];
	/* line 5 */
	uint8_t bit5[BITBANG_SLOT];
	uint8_t on_time_bit5[PADD4(ON_TIME_BIT5)];
	/* line 7 */
	uint8_t bit8[BITBANG_SLOT];
	uint8_t on_time_bit8[PADD4(ON_TIME_BIT8)];

} row_data_t;

typedef struct
{
	row_data_t rows[N_ROWS];
	uint8_t *bits[N_ROWS][BIT_DEPTH];

} frame_t;


static int cur_frame = 0;
static frame_t *frames[2];

static int row_bits(int row)
{
#if N_ROWS == 8

	int addr = 0;
	if (row&1) addr |= BIT_A;
	if (row&2) addr |= BIT_B;
	if (row&4) addr |= BIT_C;
	return addr;

#else
#error "row bits needs to be updated to accomodate a different number of rows"
#endif
}


static void init_on_time(uint8_t buf[], int size, int on_time, int bits)
{
	int i;
	assert(on_time <= size);

	for (i=0; i<on_time; i++)
		buf[DMA_ORDER(i)] = bits;

	for (i=on_time; i<size; i++)
		buf[DMA_ORDER(i)] = bits | BIT_NOT_OUTPUT_ENABLE;
}

static void init_bits(uint8_t *buf, size_t size, int old_bits, int new_bits)
{
	assert(size == N_COLUMNS);
	int i;

	for (i=0; i<BITBANG_ON_TIME; i++)
		buf[DMA_ORDER(i)] = old_bits;

	for (i=BITBANG_ON_TIME; i<size; i++)
		buf[DMA_ORDER(i)] = new_bits | BIT_NOT_OUTPUT_ENABLE;

	buf[DMA_ORDER(size-1)] |= BIT_LATCH;
}

static void init_row_data(row_data_t *data, int addr)
{
	int addr_bits = row_bits(addr), prev_addr_bits = row_bits( (addr+7) & 7 );

	memset(data, 0xff, sizeof(row_data_t));
	memset(data->off_prev_row, prev_addr_bits | BIT_NOT_OUTPUT_ENABLE, sizeof(data->off_prev_row));
	memset(data->off_cur_row, addr_bits | BIT_NOT_OUTPUT_ENABLE, sizeof(data->off_cur_row));

	init_on_time(data->on_time_bit15,  sizeof(data->on_time_bit15),  ON_TIME_BIT15,  addr_bits);
	init_on_time(data->on_time_bit14,  sizeof(data->on_time_bit14),  ON_TIME_BIT14,  addr_bits);
	init_on_time(data->on_time_bit13,  sizeof(data->on_time_bit13),  ON_TIME_BIT13,  addr_bits);
	init_on_time(data->on_time_bit12,  sizeof(data->on_time_bit12),  ON_TIME_BIT12,  addr_bits);
	init_on_time(data->on_time_bit11,  sizeof(data->on_time_bit11),  ON_TIME_BIT11,  addr_bits);
	init_on_time(data->on_time_bit11b, sizeof(data->on_time_bit11b), ON_TIME_BIT11B, addr_bits);
	init_on_time(data->on_time_bit10,  sizeof(data->on_time_bit10),  ON_TIME_BIT10,  addr_bits);
	init_on_time(data->on_time_bit9,   sizeof(data->on_time_bit9),   ON_TIME_BIT9,   addr_bits);
	init_on_time(data->on_time_bit8,   sizeof(data->on_time_bit8),   ON_TIME_BIT8,   addr_bits);
	init_on_time(data->on_time_bit7,   sizeof(data->on_time_bit7),   ON_TIME_BIT7,   addr_bits);
	init_on_time(data->on_time_bit6,   sizeof(data->on_time_bit6),   ON_TIME_BIT6,   addr_bits);
	init_on_time(data->on_time_bit5,   sizeof(data->on_time_bit5),   ON_TIME_BIT5,   addr_bits);
	init_on_time(data->on_time_bit4,   sizeof(data->on_time_bit4),   ON_TIME_BIT4,   addr_bits);

	init_bits(data->bit15, sizeof(data->bit15), addr_bits|BIT_NOT_OUTPUT_ENABLE, addr_bits);
	init_bits(data->bit14, sizeof(data->bit14), addr_bits, addr_bits);
	init_bits(data->bit13, sizeof(data->bit13), addr_bits, addr_bits);
	init_bits(data->bit12, sizeof(data->bit12), addr_bits, addr_bits);
	init_bits(data->bit11, sizeof(data->bit11), addr_bits, addr_bits);
	init_bits(data->bit10, sizeof(data->bit10), addr_bits, addr_bits);
	init_bits(data->bit9,  sizeof(data->bit9),  addr_bits, addr_bits);
	init_bits(data->bit8,  sizeof(data->bit8),  addr_bits, addr_bits);
	init_bits(data->bit7,  sizeof(data->bit7),  addr_bits, addr_bits);
	init_bits(data->bit6,  sizeof(data->bit6),  addr_bits, addr_bits);
	init_bits(data->bit5,  sizeof(data->bit5),  addr_bits, addr_bits);
	init_bits(data->bit4,  sizeof(data->bit4),  addr_bits, addr_bits);
}

static void clear_frame(frame_t *frame)
{
	int i,j,k;
	for (i=0; i<N_ROWS; i++)
		for (j=0; j<BIT_DEPTH; j++)
			for (k=0; k<N_COLUMNS; k++)
			{
				frame->bits[i][j][k] &=~ MASK_RGB;
//				frame->bits[i][j][DMA_ORDER(k)] &=~ MASK_RGB; // DMA_ORDER doesn't matter here
			}
}

/* returns total intensity */
uint32_t driver_hub75_render(int brightness, Color* fb)
{
	uint32_t total_intensity = 0;
	if (brightness < 0)
		brightness = 0;
	else if (brightness > 65535)
		brightness = 65535;

 	cur_frame ^= 1;
	frame_t *f = frames[cur_frame];
	clear_frame(f);

	int i,j,k;
	if (fb != NULL)
	for (i=0; i<N_ROWS; i++)
	{
		for (k=0; k<N_COLUMNS; k++)
		{
			/* use DMA_ORDER(k) for our reads instead of writes because it is symmetric
			 * and this is faster */
			Color *c = &fb[i*CONFIG_HUB75_WIDTH+31-DMA_ORDER(k)];

			uint32_t r = valToPwm12(c->RGB[3], brightness),
			         g = valToPwm12(c->RGB[2], brightness),
			         b = valToPwm12(c->RGB[1], brightness);

			total_intensity += r+b+g;

			for (j=0; j<BIT_DEPTH; j++)
			{
				uint32_t bit = 1<<j, val = 0;
				if (r & bit)
					val |= BIT_RED;
				if (g & bit)
					val |= BIT_GREEN;
				if (b & bit)
					val |= BIT_BLUE;

				f->bits[i][j][k] |= val;
			}
		}
	}

	i2sparallel_flipBuffer(cur_frame);
	return total_intensity;
}

static frame_t *create_frame_data(void)
{
	frame_t *frame = heap_caps_calloc(sizeof(frame_t), 1, MALLOC_CAP_DMA);
	int i;
	for(i=0; i<N_ROWS; i++)
	{
		init_row_data(&frame->rows[i], i);
#if BIT_DEPTH != 12
#error "code expects BIT_DEPTH == 12"
#endif
		frame->bits[i][11] = frame->rows[i].bit15;
		frame->bits[i][10] = frame->rows[i].bit14;
		frame->bits[i][ 9] = frame->rows[i].bit13;
		frame->bits[i][ 8] = frame->rows[i].bit12;
		frame->bits[i][ 7] = frame->rows[i].bit11;
		frame->bits[i][ 6] = frame->rows[i].bit10;
		frame->bits[i][ 5] = frame->rows[i].bit9;
		frame->bits[i][ 4] = frame->rows[i].bit8;
		frame->bits[i][ 3] = frame->rows[i].bit7;
		frame->bits[i][ 2] = frame->rows[i].bit6;
		frame->bits[i][ 1] = frame->rows[i].bit5;
		frame->bits[i][ 0] = frame->rows[i].bit4;
	}
	return frame;
}

#define RANGE_SIZE(s, from, to) ( (intptr_t)s.to + sizeof(s.to) - (intptr_t)s.from )
#define BUFFER_RANGE(s, from, to) ( (i2s_parallel_buffer_desc_t) { .memory = s.from, .size = RANGE_SIZE(s, from, to) } )
#define END_BUFFER ( (i2s_parallel_buffer_desc_t) { .memory = NULL, .size = 0 } )

#define N_DMA_DESCRIPTORS ( N_ROWS * 14 + 1 )

i2s_parallel_buffer_desc_t *create_dma_descriptors(frame_t *frame)
{
	i2s_parallel_buffer_desc_t *desc = malloc( sizeof(i2s_parallel_buffer_desc_t) * N_DMA_DESCRIPTORS );

	int i, cur = 0;

	for (i=0; i<N_ROWS; i++)
	{
		desc[ cur++ ] = BUFFER_RANGE(frame->rows[i], off_prev_row, bit11);
		desc[ cur++ ] = BUFFER_RANGE(frame->rows[i], on_time_bit11b, on_time_bit4);
	}

	for (i=0; i<N_ROWS; i++)
	{
		desc[ cur++ ] = BUFFER_RANGE(frame->rows[i], off_prev_row, on_time_bit12);
		desc[ cur++ ] = BUFFER_RANGE(frame->rows[i], bit10, on_time_bit6);
	}

	for (i=0; i<N_ROWS; i++)
	{
		desc[ cur++ ] = BUFFER_RANGE(frame->rows[i], off_prev_row, on_time_bit11);
	}

	for (i=0; i<N_ROWS; i++)
	{
		desc[ cur++ ] = BUFFER_RANGE(frame->rows[i], off_prev_row, on_time_bit12);
		desc[ cur++ ] = BUFFER_RANGE(frame->rows[i], bit9, on_time_bit7);
	}

	for (i=0; i<N_ROWS; i++)
	{
		desc[ cur++ ] = BUFFER_RANGE(frame->rows[i], off_prev_row, on_time_bit11);
	}

	for (i=0; i<N_ROWS; i++)
	{
		desc[ cur++ ] = BUFFER_RANGE(frame->rows[i], off_prev_row, on_time_bit12);
		desc[ cur++ ] = BUFFER_RANGE(frame->rows[i], bit10, on_time_bit10);
		desc[ cur++ ] = BUFFER_RANGE(frame->rows[i], bit5, on_time_bit5);
	}

	for (i=0; i<N_ROWS; i++)
	{
		desc[ cur++ ] = BUFFER_RANGE(frame->rows[i], off_prev_row, on_time_bit11);
	}

	for (i=0; i<N_ROWS; i++)
	{
		desc[ cur++ ] = BUFFER_RANGE(frame->rows[i], off_prev_row, on_time_bit12);
		desc[ cur++ ] = BUFFER_RANGE(frame->rows[i], bit8, on_time_bit8);
	}

	desc[ cur++ ] = END_BUFFER;
	assert(cur == N_DMA_DESCRIPTORS);

	return desc;
}

#ifdef DRIVER_HUB75_DMA_DATA_TEST

#define PLOT_WIDTH (128)
static void print_dma_output(i2s_parallel_buffer_desc_t desc[])
{
	size_t len=0;
	int bit,i,j,cur,chunk;
	for(i=0; desc[i].memory; i++)
		len += desc[i].size;

	char repr[]="RGBabcLE";

	for(chunk=0; chunk<len; chunk+=PLOT_WIDTH)
	{
		for(bit=0; bit<8; bit++)
		{
			cur = 0;
			for(i=0; desc[i].memory; i++)
			{
				if (cur+desc[i].size > chunk)
				{
					uint8_t *buf = (uint8_t *)desc[i].memory;
					for (j=0; j<desc[i].size; j++)
						if (cur+j >= chunk && cur+j < chunk+PLOT_WIDTH)
							printf("%c", ((buf[DMA_ORDER(j)]>>bit)&1)?repr[bit]:' ');
				}
				cur += desc[i].size;
				if (cur > chunk+PLOT_WIDTH)
					break;
			}
			printf("|\n");
		}
		for(i=0; i<PLOT_WIDTH; i++)
			printf(".");
		printf("\n");
	}
}

static void shift(uint8_t reg[N_COLUMNS], int v)
{
	int i;
	for(i=0; i<N_COLUMNS-1; i++)
		reg[i] = reg[i+1];
	reg[N_COLUMNS-1] = v;
}

static void update_row(uint32_t accum[N_COLUMNS][3], uint8_t reg[N_COLUMNS])
{
	int x;
	for (x=0; x<N_COLUMNS; x++)
	{
		if (reg[x]&BIT_RED)
			accum[x][0]++;
		if (reg[x]&BIT_GREEN)
			accum[x][1]++;
		if (reg[x]&BIT_BLUE)
			accum[x][2]++;
	}
}

static int get_row(uint8_t v)
{
#if N_ROWS == 8

	int row = 0;
	if (v&BIT_A)
		row |= 1;
	if (v&BIT_B)
		row |= 2;
	if (v&BIT_C)
		row |= 4;
	return row;

#else
#error "row bits needs to be updated to accomodate a different number of rows"
#endif
}

#define minsizeof(a, b) ( sizeof(a)<sizeof(b)?sizeof(a):sizeof(b) )
static void sym_dma(i2s_parallel_buffer_desc_t desc[], uint32_t accum[N_ROWS][N_COLUMNS][3])
{
	uint8_t reg[N_COLUMNS];
	uint8_t latch[N_COLUMNS];
	uint8_t out[N_COLUMNS];
	memset(reg, 0, sizeof(reg));

	int i,j;
	for(i=0; desc[i].memory; i++)
		for(j=0; j<desc[i].size; j++)
		{
			int v = ((uint8_t *)desc[i].memory)[DMA_ORDER(j)];
			shift(reg, (v & MASK_RGB));
			if (v & BIT_LATCH)
				memcpy(latch, reg, minsizeof(latch, reg));
			if (v & BIT_NOT_OUTPUT_ENABLE)
				memcpy(out, latch, minsizeof(out, latch));
			if ( !(v & BIT_NOT_OUTPUT_ENABLE) )
				update_row(accum[get_row(v)], out);
		}
}

static void print_accum(uint32_t accum[N_ROWS][N_COLUMNS][3])
{
	int i,j,k;
	printf("-------------\n");
	for(i=0; i<N_ROWS; i++)
	{
		for(j=0; j<3; j++)
		{
			printf("%c ", "RGB"[j]);
			for(k=0; k<N_COLUMNS; k++)
			{
				printf("%4x ", (unsigned int)accum[i][k][j]);
			}
			printf("\n");
		}
		printf("\n");
	}
}

void fill_semirand(uint32_t randbits[N_ROWS][N_COLUMNS][3], frame_t *frame)
{
	int i,j,k;
	for (i=0; i<N_ROWS; i++)
		for (k=0; k<N_COLUMNS; k++)
		{
			int r = rand()&((1<<BIT_DEPTH)-1);
			int g = rand()&((1<<BIT_DEPTH)-1);
			int b = rand()&((1<<BIT_DEPTH)-1);
			randbits[i][k][0]=r;
			randbits[i][k][1]=g;
			randbits[i][k][2]=b;
			for (j=0; j<BIT_DEPTH; j++)
			{
				if ( (1<<j) & r )
					frame->bits[i][j][DMA_ORDER(k)] |= BIT_RED;
				if ( (1<<j) & g )
					frame->bits[i][j][DMA_ORDER(k)] |= BIT_GREEN;
				if ( (1<<j) & b )
					frame->bits[i][j][DMA_ORDER(k)] |= BIT_BLUE;
			}
		}
}


#define N_RAND_TESTS 100
void do_test(i2s_parallel_buffer_desc_t desc[], frame_t *frame)
{
	srand(42);
	uint32_t randbits[N_ROWS][N_COLUMNS][3];
	uint32_t accum[N_ROWS][N_COLUMNS][3];

	int i;
	for (i=0; i<N_RAND_TESTS; i++)
	{
		clear_frame(frame);
		fill_semirand(randbits, frame);
		memset(accum, 0, sizeof(accum));
		sym_dma(desc, accum);
		assert(memcmp(randbits, accum, minsizeof(randbits, accum))==0);
	}
}


void do_test_old(i2s_parallel_buffer_desc_t desc[], frame_t *frame)
{
	print_dma_output(desc);

	uint32_t accum[N_ROWS][N_COLUMNS][3];

	int i,j,k;
	for (i=0; i<N_ROWS; i++)
		for (j=0; j<BIT_DEPTH; j++)
		{
			clear_frame(frame);
			for (k=0; k<N_COLUMNS; k++)
				frame->bits[i][j][DMA_ORDER(k)] |= MASK_RGB;
			memset(accum, 0, sizeof(accum));
			sym_dma(desc, accum);
			print_accum(accum);
		}
}

void do_test_count(i2s_parallel_buffer_desc_t desc[])
{
	int i;

	unsigned int sum = 0;
	for(i=0; desc[i].memory; i++)
		sum += desc[i].size;

	printf("net. duty cycle: %u/%u = %lf\n", 4095*8, sum, (double)(4095*8)/(double)sum);
}

#endif // DRIVER_HUB75_DMA_DATA_TEST

void driver_hub75_init_bits(void)
{
	frames[0] = create_frame_data();
	frames[1] = create_frame_data();

	i2s_parallel_buffer_desc_t *dma_desc_0 = create_dma_descriptors(frames[0]);
	i2s_parallel_buffer_desc_t *dma_desc_1 = create_dma_descriptors(frames[1]);

	i2sparallel_init(dma_desc_0, dma_desc_1);

#ifdef DRIVER_HUB75_DMA_DATA_TEST
	do_test(dma_desc_0, frames[0]);
	do_test_count(dma_desc_0);
#endif // DRIVER_HUB75_DMA_DATA_TEST

	free(dma_desc_0);
	free(dma_desc_1);
}

#ifdef DRIVER_HUB75_DMA_DATA_TEST

int main(int argc, char *argv[])
{
	driver_hub75_init_bits();
}

#endif // DRIVER_HUB75_DMA_DATA_TEST


#endif // CONFIG_DRIVER_HUB75_ENABLE
