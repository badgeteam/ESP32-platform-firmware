#ifndef DRIVER_EINK_DEV_H
#define DRIVER_EINK_DEV_H

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>

#include "driver_eink_types.h"

/** the number of horizontal pixels
 * @note the display is rotated 90 degrees
 */
#define DISP_SIZE_X 128

/** the number of vertical pixels
 * @note the display is rotated 90 degrees
 */
#define DISP_SIZE_Y 296

/** the number of bytes in a pixel row
 * @note the display is rotated 90 degrees
 */
#define DISP_SIZE_X_B ((DISP_SIZE_X + 7) >> 3)

/** the currently initialized display type */
extern enum driver_eink_dev_t driver_eink_dev_type;

__BEGIN_DECLS

/** Initialize the SPI bus to the eink display.
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_eink_dev_init(enum driver_eink_dev_t dev_type);

/** Send reset to the device
 * @return ESP_OK on success; any other value indicates an error
 */
extern esp_err_t driver_eink_dev_reset(void);

/** returns the busy-status of the eink-display
 * @return the status
 */
extern bool driver_eink_dev_is_busy(void);

/** wait for display to become ready
 */
extern void driver_eink_dev_busy_wait(void);

/** write an spi data byte to the display
 * @param data the byte to write
 */
extern void driver_eink_dev_write_byte(uint8_t data);

/** write an spi command byte to the display (without any data bytes)
 * @param command the byte to write
 */
extern void driver_eink_dev_write_command(uint8_t command);

/** helper method: write command with 1 parameter */
static inline void driver_eink_dev_write_command_p1(uint8_t command, uint8_t para1)
{
	driver_eink_dev_write_command(command);
	driver_eink_dev_write_byte(para1);
}

/** helper method: write command with 2 parameters */
static inline void driver_eink_dev_write_command_p2(uint8_t command, uint8_t para1,
									  uint8_t para2)
{
	driver_eink_dev_write_command(command);
	driver_eink_dev_write_byte(para1);
	driver_eink_dev_write_byte(para2);
}

/** helper method: write command with 3 parameters */
static inline void driver_eink_dev_write_command_p3(uint8_t command, uint8_t para1,
									  uint8_t para2, uint8_t para3)
{
	driver_eink_dev_write_command(command);
	driver_eink_dev_write_byte(para1);
	driver_eink_dev_write_byte(para2);
	driver_eink_dev_write_byte(para3);
}

/** helper method: write command with 4 parameters */
static inline void driver_eink_dev_write_command_p4(uint8_t command, uint8_t para1,
									  uint8_t para2, uint8_t para3,
									  uint8_t para4)
{
	driver_eink_dev_write_command(command);
	driver_eink_dev_write_byte(para1);
	driver_eink_dev_write_byte(para2);
	driver_eink_dev_write_byte(para3);
	driver_eink_dev_write_byte(para4);
}

/** write command with `datalen` data bytes */
void driver_eink_dev_write_command_stream(uint8_t command, const uint8_t *data,
										 unsigned int datalen);

/** write command with `datalen` data dwords */
void driver_eink_dev_write_command_stream_u32(uint8_t command, const uint32_t *data,
										 unsigned int datalen);

__END_DECLS

#endif // DRIVER_EINK_DEV_H
