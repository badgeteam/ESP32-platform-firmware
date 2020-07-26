#pragma once

#include <esp_err.h>
#include "compositor.h"
#include "color.h"

#define HUB75_WIDTH CONFIG_HUB75_WIDTH
#define HUB75_HEIGHT CONFIG_HUB75_HEIGHT
#define HUB75_BUFFER_SIZE HUB75_WIDTH*HUB75_HEIGHT*sizeof(Color)

#ifdef __cplusplus
extern "C" {
#endif


esp_err_t driver_hub75_init(void);
void driver_hub75_set_brightness(int brightness_val);
void driver_hub75_set_framerate(int framerate_val);
void driver_hub75_switch_buffer(uint8_t* buffer);

Color* getFrameBuffer();

#ifdef __cplusplus
}
#endif
