#ifndef _DEMO1_H_
#define _DEMO1_H_

#include "Adafruit_PixelDust.h"
#include "../../driver_display_hub75/include/compositor.h"
#include <esp_err.h>

esp_err_t particle_init();
void particle_initSim(int flakes);
void particle_disp();
void particle_setBuffer(Color* framebuffer);
bool particle_status();
void particle_disable();

#endif
