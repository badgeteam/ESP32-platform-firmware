#ifndef DRIVER_DOTFLIPPER_H
#define DRIVER_DOTFLIPPER_H

#include <stdint.h>
#include <esp_err.h>

__BEGIN_DECLS

#define DOTFLIPPER_PANELWIDTH  32         // in dots
#define DOTFLIPPER_PANELHEIGHT 16         // in dots

#define DOTFLIPPER_BUFFER_SIZE (CONFIG_DOTFLIPPER_WIDTH*DOTFLIPPER_PANELWIDTH*CONFIG_DOTFLIPPER_HEIGHT*DOTFLIPPER_PANELHEIGHT) / 8

#define DOTFLIPPER_WIDTH CONFIG_DOTFLIPPER_WIDTH*DOTFLIPPER_PANELWIDTH
#define DOTFLIPPER_HEIGHT CONFIG_DOTFLIPPER_HEIGHT*DOTFLIPPER_PANELHEIGHT
#define DOTFLIPPER_SINGLEPANEL_DELAY 400    // in microseconds if there is only one panel

#define DOTFLIPPER_NUMPANELS CONFIG_DOTFLIPPER_WIDTH*CONFIG_DOTFLIPPER_HEIGHT

extern esp_err_t driver_dotflipper_init(void);
extern esp_err_t driver_dotflipper_write(const uint8_t *buffer);

__END_DECLS

#endif

