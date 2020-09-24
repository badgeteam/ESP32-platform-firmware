#ifndef DRIVER_MPR121_H
#define DRIVER_MPR121_H

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>

#define MPR121_TOUCHSTATUS_L 0x00
#define MPR121_TOUCHSTATUS_H 0x01
#define MPR121_FILTDATA_0L   0x04
#define MPR121_FILTDATA_0H   0x05
#define MPR121_BASELINE_0    0x1E
#define MPR121_MHDR          0x2B
#define MPR121_NHDR          0x2C
#define MPR121_NCLR          0x2D
#define MPR121_FDLR          0x2E
#define MPR121_MHDF          0x2F
#define MPR121_NHDF          0x30
#define MPR121_NCLF          0x31
#define MPR121_FDLF          0x32
#define MPR121_NHDT          0x33
#define MPR121_NCLT          0x34
#define MPR121_FDLT          0x35
#define MPR121_TOUCHTH_0     0x41
#define MPR121_RELEASETH_0   0x42
#define MPR121_DEBOUNCE      0x5B
#define MPR121_CONFIG1       0x5C
#define MPR121_CONFIG2       0x5D
#define MPR121_CHARGECURR_0  0x5F
#define MPR121_CHARGETIME_1  0x6C
#define MPR121_ECR           0x5E
#define MPR121_AUTOCONFIG0   0x7B
#define MPR121_AUTOCONFIG1   0x7C
#define MPR121_UPLIMIT       0x7D
#define MPR121_LOWLIMIT      0x7E
#define MPR121_TARGETLIMIT   0x7F
#define MPR121_GPIODIR       0x76
#define MPR121_GPIOEN        0x77
#define MPR121_GPIOSET       0x78
#define MPR121_GPIOCLR       0x79
#define MPR121_GPIOTOGGLE    0x7A
#define MPR121_SOFTRESET     0x80

__BEGIN_DECLS

typedef void (*driver_mpr121_intr_t)(void*, bool); // Interrupt handler type

struct driver_mpr121_touch_info {
	uint32_t touch_state;  // Bitmapped touch state
	uint32_t data[12];     // The electrode data    (10 bits)
	uint32_t baseline[12]; // The baseline          (8 bits)
	uint32_t touch[12];    // The touch threshold   (8 bits)
	uint32_t release[12];  // The release threshold (8 bits)
};

/**
 * Initialize internal structures and interrupt-handling for the MPR121.
 * @note This should be called before using any other methods.
 *   The only exception is driver_mpr121_set_interrupt_handler().
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_mpr121_init(void);

/**
 * Configure the mpr121 touch with new baselines.
 * @param baseline if not NULL, the MPR121 will be configured to use
 *   these baseline values for inputs 0..7.
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_mpr121_configure(const uint32_t *baseline, uint8_t press, uint8_t release);

/**
 * Configure interrupt handler for a specific pin.
 * @param pin the pin-number on the mpr121 chip.
 * @param handler the handler to be called on an interrupt.
 * @param arg the argument passed on to the handler.
 * @note It is safe to set the interrupt handler before a call to driver_mpr121_init().
 */
extern void driver_mpr121_set_interrupt_handler(uint8_t pin, driver_mpr121_intr_t handler, void *arg);

/**
 * Retrieve the mpr121 status.
 * @return the status registers; or -1 on error
 */
extern int driver_mpr121_get_interrupt_status_touch(void);
extern int driver_mpr121_get_interrupt_status_gpio(void);

/**
 * Retrieve mpr121 touch info
 * @param info touch info will be written to this structure.
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_mpr121_get_touch_info(struct driver_mpr121_touch_info *info);

/** gpio config settings */
enum driver_mpr121_gpio_config
{
	MPR121_DISABLED         = 0x00,
	MPR121_INPUT            = 0x08,
	MPR121_INPUT_PULL_DOWN  = 0x09,
	MPR121_INPUT_PULL_UP    = 0x0b,
	MPR121_OUTPUT           = 0x0c,
	MPR121_OUTPUT_LOW_ONLY  = 0x0e,
	MPR121_OUTPUT_HIGH_ONLY = 0x0f,
};

/**
 * Configure GPIO pin on the MPR121.
 * @param pin the pin-number on the mpr121 chip.
 * @param config the new gpio pin config.
 * @return 0 on success; -1 on error
 */
extern int driver_mpr121_configure_gpio(int pin, enum driver_mpr121_gpio_config config);

/**
 * Retrieve the level of a GPIO pin.
 * @param pin the pin-number on the mpr121 chip.
 * @return 0 when low; 1 when high; -1 on error
 */
extern int driver_mpr121_get_gpio_level(int pin);

/**
 * Set the level of a GPIO pin.
 * @param pin the pin-number on the mpr121 chip.
 * @param value 0 is low; 1 is high
 * @return 0 on succes; -1 on error
 */
extern int driver_mpr121_set_gpio_level(int pin, int value);

bool driver_mpr121_is_digital_output(int ele);
bool driver_mpr121_is_digital_input(int ele);
bool driver_mpr121_is_touch_input(int ele);

extern int driver_mpr121_get_touch_level(int pin);

__END_DECLS

#endif
