#ifndef DRIVER_EINK_TYPES_H
#define DRIVER_EINK_TYPES_H

#include <sdkconfig.h>

/**
 * e-ink types.
 */
/* NOTE: This value is used in nvs. Do not change order or remove elements. */
enum driver_eink_dev_t {
	DRIVER_EINK_NONE,
	DRIVER_EINK_GDEH029A1,
	DRIVER_EINK_DEPG0290B1,
};

/**
 * DRIVER_EINK_DEFAULT specifies the compile-time default display type
 */
#if defined(CONFIG_DRIVER_EINK_TYPE_DEPG0290B1)
 #define DRIVER_EINK_DEFAULT DRIVER_EINK_DEPG0290B1
#elif defined(CONFIG_DRIVER_EINK_TYPE_GDEH029A1)
 #define DRIVER_EINK_DEFAULT DRIVER_EINK_GDEH029A1
#else
 #define DRIVER_EINK_DEFAULT DRIVER_EINK_NONE
#endif

#endif // DRIVER_EINK_TYPES_H
