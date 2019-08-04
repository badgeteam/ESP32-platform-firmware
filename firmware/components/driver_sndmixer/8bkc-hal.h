#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/i2s.h"
#include "esp_sleep.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/dac.h"
#include "soc/rtc_cntl_reg.h"

void kchal_sound_start(int rate, int buffsize);
void kchal_sound_push(uint8_t *buf, int len);
void kchal_sound_stop();
void kchal_sound_mute(int doMute);

void kchal_set_volume(uint8_t new_volume);
uint8_t kchal_get_volume();
