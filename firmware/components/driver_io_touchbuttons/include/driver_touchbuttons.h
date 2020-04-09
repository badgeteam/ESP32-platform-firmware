#ifndef DRIVER_I2C_DISPLAY
#define DRIVER_I2C_DISPLAY

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>

#include "esp_intr.h"
#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "soc/touch_channel.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"

#ifdef CONFIG_TOUCH0_ENABLE
#define CONFIG_TOUCH0_ENABLED 1
#else
#define CONFIG_TOUCH0_ENABLED 0
#endif

#ifdef CONFIG_TOUCH1_ENABLE
#define CONFIG_TOUCH1_ENABLED 1
#else
#define CONFIG_TOUCH1_ENABLED 0
#endif

#ifdef CONFIG_TOUCH2_ENABLE
#define CONFIG_TOUCH2_ENABLED 1
#else
#define CONFIG_TOUCH2_ENABLED 0
#endif

#ifdef CONFIG_TOUCH3_ENABLE
#define CONFIG_TOUCH3_ENABLED 1
#else
#define CONFIG_TOUCH3_ENABLED 0
#endif

#ifdef CONFIG_TOUCH4_ENABLE
#define CONFIG_TOUCH4_ENABLED 1
#else
#define CONFIG_TOUCH4_ENABLED 0
#endif

#ifdef CONFIG_TOUCH5_ENABLE
#define CONFIG_TOUCH5_ENABLED 1
#else
#define CONFIG_TOUCH5_ENABLED 0
#endif

#ifdef CONFIG_TOUCH6_ENABLE
#define CONFIG_TOUCH6_ENABLED 1
#else
#define CONFIG_TOUCH6_ENABLED 0
#endif

#ifdef CONFIG_TOUCH7_ENABLE
#define CONFIG_TOUCH7_ENABLED 1
#else
#define CONFIG_TOUCH7_ENABLED 0
#endif

#ifdef CONFIG_TOUCH8_ENABLE
#define CONFIG_TOUCH8_ENABLED 1
#else
#define CONFIG_TOUCH8_ENABLED 0
#endif

#ifdef CONFIG_TOUCH9_ENABLE
#define CONFIG_TOUCH9_ENABLED 1
#else
#define CONFIG_TOUCH9_ENABLED 0
#endif

#define TOUCHPAD_FILTER_TOUCH_PERIOD (5)

__BEGIN_DECLS

extern esp_err_t driver_touchbuttons_init(void);
extern uint16_t get_touch_state();
extern uint16_t get_touch_value(uint8_t pad);
extern esp_err_t set_touch_handler(void (*handler)(uint16_t));

__END_DECLS

#endif // DRIVER_I2C_DISPLAY
