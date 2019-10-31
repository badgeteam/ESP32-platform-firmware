#include <sdkconfig.h>
#include "driver_i2s.h"

#ifdef CONFIG_DRIVER_SNDMIXER_ENABLE

struct Config {
	uint8_t volume;
} config;

static QueueHandle_t soundQueue;
static int soundRunning=0;

void driver_i2s_sound_start() {
	config.volume = 255;
	
	int rate     = CONFIG_DRIVER_SNDMIXER_SAMPLE_RATE;
	int buffsize = CONFIG_DRIVER_SNDMIXER_BUFFZIE;
	
	i2s_config_t cfg={
		#ifdef CONFIG_DRIVER_SNDMIXER_I2S_DAC_INTERNAL
			.mode= I2S_MODE_TX | I2S_MODE_MASTER | I2S_MODE_DAC_BUILT_IN,
		#else
			.mode= I2S_MODE_TX | I2S_MODE_MASTER,
		#endif
		.sample_rate = rate,
		.bits_per_sample = CONFIG_DRIVER_SNDMIXER_BITS_PER_SAMPLE,
		#if defined(CONFIG_DRIVER_SNDMIXER_I2S_CHANNEL_FORMAT_OR)
			.channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
		#elif defined(CONFIG_DRIVER_SNDMIXER_I2S_CHANNEL_FORMAT_OL)
			.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
		#elif defined(CONFIG_DRIVER_SNDMIXER_I2S_CHANNEL_FORMAT_AL)
			.channel_format = I2S_CHANNEL_FMT_ALL_LEFT,
		#elif defined(CONFIG_DRIVER_SNDMIXER_I2S_CHANNEL_FORMAT_AR)
			.channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
		#else
			.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
		#endif
		#ifdef CONFIG_DRIVER_SNDMIXER_I2S_DAC_INTERNAL
			.communication_format = I2S_COMM_FORMAT_I2S_MSB,
		#elif defined(CONFIG_DRIVER_SNDMIXER_I2S_DAC_EXTERNAL_MSB)
			.communication_format = I2S_COMM_FORMAT_I2S_MSB | I2S_COMM_FORMAT_I2S,
		#elif defined(CONFIG_DRIVER_SNDMIXER_I2S_DAC_EXTERNAL_LSB)
			.communication_format = I2S_COMM_FORMAT_I2S_LSB | I2S_COMM_FORMAT_I2S,
		#else
			.communication_format = I2S_COMM_FORMAT_I2S,
		#endif
		.intr_alloc_flags=0,
		.dma_buf_count=4,
		.dma_buf_len=buffsize/4
	};
	
	#ifdef DRIVER_SNDMIXER_I2S_PORT1
		const i2s_port_t port = 1;
	#else
		const i2s_port_t port = 0;
	#endif
	
	i2s_driver_install(port, &cfg, 4, &soundQueue);
	i2s_set_sample_rates(port, cfg.sample_rate);
	#ifdef CONFIG_DRIVER_SNDMIXER_I2S_DAC_INTERNAL
		i2s_set_pin(port, NULL);
		#ifdef CONFIG_DRIVER_SNDMIXER_I2S_INTERNAL_DAC_BOTH
			i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);
		#elif defined(CONFIG_DRIVER_SNDMIXER_I2S_INTERNAL_DAC_RIGHT)
			i2s_set_dac_mode(I2S_DAC_CHANNEL_RIGHT_EN);
		#elif defined(CONFIG_DRIVER_SNDMIXER_I2S_INTERNAL_DAC_LEFT)
			i2s_set_dac_mode(I2S_DAC_CHANNEL_LEFT_EN);
		#else
			i2s_set_dac_mode(I2S_DAC_CHANNEL_DISABLE);
		#endif
	#else
		static const i2s_pin_config_t pin_config = {
			.bck_io_num   = CONFIG_DRIVER_SNDMIXER_PIN_BCK,
			.ws_io_num    = CONFIG_DRIVER_SNDMIXER_PIN_WS,
			.data_out_num = CONFIG_DRIVER_SNDMIXER_PIN_DATA_OUT,
			.data_in_num  = I2S_PIN_NO_CHANGE
		};
		i2s_set_pin(port, &pin_config);
	#endif
	soundRunning=1;
}

void driver_i2s_sound_stop() {
	i2s_driver_uninstall(0);
}

#define SND_CHUNKSZ 32
void driver_i2s_sound_push(uint8_t *buf, int len) {
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
		
		#ifdef DRIVER_SNDMIXER_I2S_PORT1
			const i2s_port_t port = 1;
		#else
			const i2s_port_t port = 0;
		#endif
		
		i2s_write_bytes(port, (char*)tmpb, plen*4, portMAX_DELAY);
		i+=plen;
	}
}

void driver_i2s_set_volume(uint8_t new_volume) {
	//xSemaphoreTake(configMux, portMAX_DELAY);
	config.volume=new_volume;
	//xSemaphoreGive(configMux);
}

uint8_t driver_i2s_get_volume() {
	return config.volume;
}

void driver_i2s_sound_mute(int doMute) {
	if (doMute) {
		dac_i2s_disable();
	} else {
		dac_i2s_enable();
	}
}

#endif
