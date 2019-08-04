#include <sdkconfig.h>
#include "8bkc-hal.h"

#ifdef CONFIG_DRIVER_SNDMIXER_ENABLE

struct Config {
	uint8_t volume;
} config;

static QueueHandle_t soundQueue;
static int soundRunning=0;

#define CONFIG_DRIVER_SNDMIXER_I2S_NUM 1

void kchal_sound_start(int rate, int buffsize) {
	config.volume = 255;
	
#if 0 //odroid go
	i2s_config_t cfg={
		.mode=I2S_MODE_DAC_BUILT_IN|I2S_MODE_TX|I2S_MODE_MASTER,
		.sample_rate=rate,
		.bits_per_sample=16,
		.channel_format=I2S_CHANNEL_FMT_RIGHT_LEFT,
		.communication_format=I2S_COMM_FORMAT_I2S_MSB,
		.intr_alloc_flags=0,
		.dma_buf_count=4,
		.dma_buf_len=buffsize/4
	};
	
	i2s_driver_install(0, &cfg, 4, &soundQueue);
	i2s_set_sample_rates(0, cfg.sample_rate);
	i2s_set_pin((i2s_port_t)0, NULL);
	i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);
#endif
	
	i2s_config_t cfg={
		.mode=I2S_MODE_TX|I2S_MODE_MASTER,
		.sample_rate=rate,
		.bits_per_sample=16,
		.channel_format=I2S_CHANNEL_FMT_RIGHT_LEFT,
		.communication_format=I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_LSB,
		.intr_alloc_flags=0,
		.dma_buf_count=4,
		.dma_buf_len=buffsize/4
	};
	
	static const i2s_pin_config_t pin_config = {
		.bck_io_num = 13,
		.ws_io_num = 15,
		.data_out_num = 2,
		.data_in_num = I2S_PIN_NO_CHANGE
	};
	
	i2s_driver_install(CONFIG_DRIVER_SNDMIXER_I2S_NUM, &cfg, 4, &soundQueue);
	i2s_set_sample_rates(CONFIG_DRIVER_SNDMIXER_I2S_NUM, cfg.sample_rate);
	i2s_set_pin(CONFIG_DRIVER_SNDMIXER_I2S_NUM, &pin_config);
	
	soundRunning=1;
}


#define SND_CHUNKSZ 32
void kchal_sound_push(uint8_t *buf, int len) {
	uint32_t tmpb[SND_CHUNKSZ];
	int i=0;
	while (i<len) {
		int plen=len-i;
		if (plen>SND_CHUNKSZ) plen=SND_CHUNKSZ;
		for (int j=0; j<plen; j++) {
			int s=((((int)buf[i+j])-128)*config.volume); //Make [-128,127], multiply with volume
			s=(s>>8)+128; //divide off volume max, get back to [0-255]
			if (s>255) s=255;
			if (s<0) s=0;
			tmpb[j]=((s)<<8)+((s)<<24);
		}
		i2s_write_bytes(CONFIG_DRIVER_SNDMIXER_I2S_NUM, (char*)tmpb, plen*4, portMAX_DELAY);
		i+=plen;
	}
}

void kchal_sound_stop() {
	i2s_driver_uninstall(0);
}

void kchal_sound_mute(int doMute) {
	if (doMute) {
		dac_i2s_disable();
	} else {
		dac_i2s_enable();
	}
}


void kchal_set_volume(uint8_t new_volume) {
	//xSemaphoreTake(configMux, portMAX_DELAY);
	config.volume=new_volume;
	//xSemaphoreGive(configMux);
}

uint8_t kchal_get_volume() {
	return config.volume;
}

#endif
