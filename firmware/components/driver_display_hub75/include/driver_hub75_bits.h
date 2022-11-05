#ifndef DRIVER_HUB75_BITS_H
#define DRIVER_HUB75_BITS_H

#include "color.h"

#ifdef __cplusplus
extern "C" {
#endif

/* returns total intensity */
uint32_t driver_hub75_render(int brightness, Color* fb);
void driver_hub75_init_bits(void);

#ifdef __cplusplus
}
#endif

#endif //DRIVER_HUB75_BITS_H


