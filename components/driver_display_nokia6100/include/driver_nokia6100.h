#ifndef DRIVER_NOKIA6100_H
#define DRIVER_NOKIA6100_H

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>

#define NOKIA6100_WIDTH  132
#define NOKIA6100_HEIGHT 132
#define NOKIA6100_BUFFER_SIZE NOKIA6100_WIDTH * NOKIA6100_HEIGHT * 2

__BEGIN_DECLS

//EPSON Controller Definitions
#define DISON       0xAF    // Display on
#define DISOFF      0xAE    // Display off
#define DISNOR      0xA6    // Normal display
#define DISINV      0xA7    // Inverse display
#define SLPIN       0x95    // Sleep in
#define SLPOUT      0x94    // Sleep out
#define COMSCN      0xBB    // Common scan direction
#define DISCTL      0xCA    // Display control
#define PASET       0x75    // Page address set
#define CASET       0x15    // Column address set
#define DATCTL      0xBC    // Data scan direction, etc.
#define RGBSET8     0xCE    // 256-color position set
#define RAMWR       0x5C    // Writing to memory
#define RAMRD       0x5D    // Reading from memory
#define PTLIN       0xA8    // Partial display in
#define PTLOUT      0xA9    // Partial display out
#define RMWIN       0xE0    // Read and modify write
#define RMWOUT      0xEE    // End
#define ASCSET      0xAA    // Area scroll set
#define SCSTART     0xAB    // Scroll start set
#define OSCON       0xD1    // Internal oscillation on
#define OSCOFF      0xD2    // Internal osciallation off
#define PWRCTR      0x20    // Power control
#define VOLCTR      0x81    // Electronic volume control
#define VOLUP       0xD6    // Increment electronic control by 1
#define VOLDOWN     0xD7    // Decrement electronic control by 1
#define TMPGRD      0x82    // Temperature gradient set
#define EPCTIN      0xCD    // Control EEPROM
#define EPCOUT      0xCC    // Cancel EEPROM control
#define EPMWR       0xFC    // Write into EEPROM
#define EPMRD       0xFD    // Read from EEPROM
#define EPSRRD1     0x7C    // Read register 1
#define EPSRRD2     0x7D    // Read register 2
#define NOP         0x25    // No op

//PHILLIPS Controller Definitions
#define NOPP        0x00    // No operation
#define BSTRON      0x03    // Booster voltage on
#define SLEEPIN     0x10    // Sleep in
#define SLEEPOUT    0x11    // Sleep out
#define NORON       0x13    // Normal display mode on
#define INVOFF      0x20    // Display inversion off
#define INVON       0x21    // Display inversion on
#define SETCON      0x25    // Set contrast
#define DISPOFF     0x28    // Display off
#define DISPON      0x29    // Display on
#define CASETP      0x2A    // Column address set
#define PASETP      0x2B    // Page address set
#define RAMWRP      0x2C    // Memory write
#define RGBSET      0x2D    // Color set
#define MADCTL      0x36    // Memory data access control
#define PCOLMOD     0x3A    // Interface pixel format
#define DISCTR      0xB9    // Super frame inversion
#define EC          0xC0    // Internal or external oscillator

extern esp_err_t driver_nokia6100_init(void);
extern esp_err_t driver_nokia6100_set_backlight(bool state);
extern esp_err_t driver_nokia6100_write(const uint8_t *data);
extern esp_err_t driver_nokia6100_write_partial(const uint8_t *buffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

__END_DECLS

#endif // DRIVER_NOKIA6100_H
