# Writing custom C drivers
If you need low-level support for hardware that isn't available yet, you can write your own drivers, and can expose them to the Python app layer.
 * Create the folder `/firmware/components/driver_<category>_<name>`.
 * In this folder, create files `component.mk`, `Kconfig`, and `driver_<name>.c`. Kconfig allows you to add configurable switches and variables from the `./config.sh` step. The driver source file exposes an initialisation function that will be called upon boot. Have a look at e.g. `/firmware/components/driver_bus_i2c` to see how to populate these files.
 * In `/main/platform.c:platform_init()`, add `INIT_DRIVER(<name>)` to have your driver actually initialise during boot.
 * Add your driver's header directory to `firmware/micropython/component.mk`, e.g. `MP_EXTRA_INC += -I$(PROJECT_PATH)/components/driver_<category>_<name>/include`.
 * Add python bindings to your driver by creating `components/micropython/esp32/mod<name>.c` (see e.g. modi2c.c).
 * Tell micropython about your bindings by adding the following to `firmware/micropython/component.mk`:
```
ifdef CONFIG_DRIVER_<NAME>_ENABLE
SRC_C += esp32/mod<name>.c
endif
```
 * Add the following to `components/micropython/esp32/mpconfigport.h` to add the module symbols to the python environment (replace i2c with your name):
```
#ifdef CONFIG_DRIVER_I2C_ENABLE
extern const struct _mp_obj_module_t i2c_module;
#endif
```
```
#ifdef CONFIG_DRIVER_I2C_ENABLE
#define BUILTIN_MODULE_I2C { MP_OBJ_NEW_QSTR(MP_QSTR_i2c), (mp_obj_t)&i2c_module },
#else
#define BUILTIN_MODULE_I2C
#endif
```
```
(to the define called MICROPY_PORT_BUILTIN_MODULES, add the following line after the other drivers):
BUILTIN_MODULE_I2C \
```