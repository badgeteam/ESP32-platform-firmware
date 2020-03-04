#include <sdkconfig.h>

#ifdef CONFIG_DRIVER_HUB75_ENABLE

// Copyright 2017 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <soc/i2s_struct.h>

#include "freertos/FreeRTOS.h"
#include "soc/i2s_struct.h"
#include "soc/i2s_reg.h"
#include "driver/periph_ctrl.h"
#include "soc/io_mux_reg.h"
#include "rom/lldesc.h"
#include "esp_heap_caps.h"
#include "include/val2pwm.h"
#include "include/i2s_parallel.h"

#define hw I2S1

typedef struct {
    volatile lldesc_t *dmadesc_a, *dmadesc_b;
    int desccount_a, desccount_b;
} i2s_parallel_state_t;

static i2s_parallel_state_t *i2s_state[2]={NULL, NULL};
int gpio_bus[32] = {CONFIG_PIN_NUM_HUB75_R0,
                    CONFIG_PIN_NUM_HUB75_G0,
                    CONFIG_PIN_NUM_HUB75_B0,
                    CONFIG_PIN_NUM_HUB75_A,
                    CONFIG_PIN_NUM_HUB75_B,
                    CONFIG_PIN_NUM_HUB75_C,
                    CONFIG_PIN_NUM_HUB75_LAT,
                    CONFIG_PIN_NUM_HUB75_OE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
int gpio_clk = CONFIG_PIN_NUM_HUB75_CLK;
int clkspeed_hz = CONFIG_HUB75_CLOCK_SPEED;
i2s_parallel_cfg_bits_t bits = I2S_PARALLEL_BITS_8;

#define DMA_MAX (4096-4)

//Calculate the amount of dma descs needed for a buffer desc
static int calc_needed_dma_descs_for(i2s_parallel_buffer_desc_t *desc) {
    int ret=0;
    for (int i=0; desc[i].memory!=NULL; i++) {
        ret+=(desc[i].size+DMA_MAX-1)/DMA_MAX;
    }
    return ret;
}

static void fill_dma_desc(volatile lldesc_t *dmadesc, i2s_parallel_buffer_desc_t *bufdesc) {
    int n=0;
    for (int i=0; bufdesc[i].memory!=NULL; i++) {
        int len=bufdesc[i].size;
        uint8_t *data=(uint8_t*)bufdesc[i].memory;
        while(len) {
            int dmalen=len;
            if (dmalen>DMA_MAX) dmalen=DMA_MAX;
            dmadesc[n].size=dmalen;
            dmadesc[n].length=dmalen;
            dmadesc[n].buf=data;
            dmadesc[n].eof=0;
            dmadesc[n].sosf=0;
            dmadesc[n].owner=1;
            dmadesc[n].qe.stqe_next=(lldesc_t*)&dmadesc[n+1];
            dmadesc[n].offset=0;
            len-=dmalen;
            data+=dmalen;
            n++;
        }
    }
    //Loop last back to first
    dmadesc[n-1].qe.stqe_next=(lldesc_t*)&dmadesc[0];
    printf("fill_dma_desc: filled %d descriptors\n", n);
}

static void gpio_setup_out(int gpio, int sig) {
    if (gpio==-1) return;
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[gpio], PIN_FUNC_GPIO);
    gpio_set_direction((gpio_num_t) gpio, GPIO_MODE_OUTPUT);
    gpio_matrix_out(gpio, sig, false, false);
}


static void dma_reset() {
    hw.lc_conf.in_rst=1; hw.lc_conf.in_rst=0;
    hw.lc_conf.out_rst=1; hw.lc_conf.out_rst=0;
}

static void fifo_reset() {
    hw.conf.rx_fifo_reset=1; hw.conf.rx_fifo_reset=0;
    hw.conf.tx_fifo_reset=1; hw.conf.tx_fifo_reset=0;
}

static int i2snum() {
    return (&hw==&I2S0)?0:1;
}

void i2sparallel_init(i2s_parallel_buffer_desc_t *bufa, i2s_parallel_buffer_desc_t *bufb) {
    //Figure out which signal numbers to use for routing
    int sig_data_base, sig_clk;
    if (&hw==&I2S0) {
        sig_data_base=I2S0O_DATA_OUT0_IDX;
        sig_clk=I2S0O_WS_OUT_IDX;
    } else {
        if (bits==I2S_PARALLEL_BITS_32) {
            sig_data_base=I2S1O_DATA_OUT0_IDX;
        } else if(bits==I2S_PARALLEL_BITS_16){
            //Because of... reasons... the 16-bit values for i2s1 appear on d8...d23
            sig_data_base=I2S1O_DATA_OUT8_IDX;
        } else {
            sig_data_base=I2S1O_DATA_OUT0_IDX;
        }
        sig_clk=I2S1O_WS_OUT_IDX;
    }

    //Route the signals
    for (int x=0; x<bits; x++) {
        gpio_setup_out(gpio_bus[x], sig_data_base+x);
    }
    //ToDo: Clk/WS may need inversion?
    gpio_setup_out(gpio_clk, sig_clk);
    gpio_matrix_out(gpio_clk, sig_clk, true, false);

    //Power on dev
    if (&hw==&I2S0) {
        periph_module_enable(PERIPH_I2S0_MODULE);
    } else {
        periph_module_enable(PERIPH_I2S1_MODULE);
    }
    //Initialize I2S dev
    hw.conf.rx_reset=1; hw.conf.rx_reset=0;
    hw.conf.tx_reset=1; hw.conf.tx_reset=0;
    dma_reset();
    fifo_reset();

    //Enable LCD mode
    hw.conf2.val=0;
    hw.conf2.lcd_en=1;

    hw.sample_rate_conf.val=0;
    hw.sample_rate_conf.rx_bits_mod=bits;
    hw.sample_rate_conf.tx_bits_mod=bits;
    hw.sample_rate_conf.rx_bck_div_num=4; //ToDo: Unsure about what this does...
    hw.sample_rate_conf.tx_bck_div_num=4;
    hw.conf2.lcd_tx_wrx2_en=1;

    hw.clkm_conf.val=0;
    hw.clkm_conf.clka_en=0;
    hw.clkm_conf.clkm_div_a=1;
    hw.clkm_conf.clkm_div_b=1;
    //We ignore the possibility for fractional division here, clkspeed_hz must round up for a fractional clock speed, must result in >= 2
    hw.clkm_conf.clkm_div_num=(40000000/clkspeed_hz)-1;

    hw.fifo_conf.val=0;
    hw.fifo_conf.rx_fifo_mod_force_en=1;
    hw.fifo_conf.tx_fifo_mod_force_en=1;
    hw.fifo_conf.tx_fifo_mod=1;
    hw.fifo_conf.tx_fifo_mod=1;
    hw.fifo_conf.rx_data_num=32; //Thresholds.
    hw.fifo_conf.tx_data_num=32;
    hw.fifo_conf.dscr_en=1;

    hw.conf1.val=0;
    hw.conf1.tx_stop_en=0;
    hw.conf1.tx_pcm_bypass=1;

    hw.conf_chan.val=0;
    hw.conf_chan.tx_chan_mod=1;
    hw.conf_chan.rx_chan_mod=1;

    //Invert ws to be active-low... ToDo: make this configurable
    hw.conf.tx_right_first=1;
    hw.conf.rx_right_first=1;

    hw.timing.val=0;

    //Allocate DMA descriptors
    i2s_state[i2snum()]=(i2s_parallel_state_t *) calloc(1, sizeof(i2s_parallel_state_t));
    i2s_parallel_state_t *st=i2s_state[i2snum()];
    st->desccount_a=calc_needed_dma_descs_for(bufa);
    st->desccount_b=calc_needed_dma_descs_for(bufb);
    st->dmadesc_a=(volatile lldesc_t *) heap_caps_calloc(st->desccount_a, sizeof(lldesc_t), MALLOC_CAP_DMA);
    st->dmadesc_b=(volatile lldesc_t *) heap_caps_calloc(st->desccount_b, sizeof(lldesc_t), MALLOC_CAP_DMA);

    //and fill them
    fill_dma_desc(st->dmadesc_a, bufa);
    fill_dma_desc(st->dmadesc_b, bufb);

    //Reset FIFO/DMA -> needed? Doesn't dma_reset/fifo_reset do this?
    hw.lc_conf.in_rst=1; hw.lc_conf.out_rst=1; hw.lc_conf.ahbm_rst=1; hw.lc_conf.ahbm_fifo_rst=1;
    hw.lc_conf.in_rst=0; hw.lc_conf.out_rst=0; hw.lc_conf.ahbm_rst=0; hw.lc_conf.ahbm_fifo_rst=0;
    hw.conf.tx_reset=1; hw.conf.tx_fifo_reset=1; hw.conf.rx_fifo_reset=1;
    hw.conf.tx_reset=0; hw.conf.tx_fifo_reset=0; hw.conf.rx_fifo_reset=0;

    //Start dma on front buffer
    hw.lc_conf.val=I2S_OUT_DATA_BURST_EN | I2S_OUTDSCR_BURST_EN | I2S_OUT_DATA_BURST_EN;
    hw.out_link.addr=((uint32_t)(&st->dmadesc_a[0]));
    hw.out_link.start=1;
    hw.conf.tx_start=1;
}


//Flip to a buffer: 0 for bufa, 1 for bufb
void i2sparallel_flipBuffer(int bufid) {
    int no=i2snum();
    if (i2s_state[no]==NULL) return;
    lldesc_t *active_dma_chain;
    if (bufid==0) {
        active_dma_chain=(lldesc_t*)&i2s_state[no]->dmadesc_a[0];
    } else {
        active_dma_chain=(lldesc_t*)&i2s_state[no]->dmadesc_b[0];
    }

    i2s_state[no]->dmadesc_a[i2s_state[no]->desccount_a-1].qe.stqe_next=active_dma_chain;
    i2s_state[no]->dmadesc_b[i2s_state[no]->desccount_b-1].qe.stqe_next=active_dma_chain;
}

#endif
