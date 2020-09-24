#ifndef DRIVER_ILI9341_H
#define DRIVER_ILI9341_H

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>

#define ILI9341_WIDTH  320
#define ILI9341_HEIGHT 240

#ifdef CONFIG_DRIVER_ILI9341_8C
	#define ILI9341_BUFFER_SIZE ILI9341_WIDTH * ILI9341_HEIGHT
#else
	#define ILI9341_BUFFER_SIZE ILI9341_WIDTH * ILI9341_HEIGHT * 2
#endif

#define ILI9341_NOP         0x00
#define ILI9341_SWRESET     0x01 // Software Reset
#define ILI9341_RDDID       0x04 // Read Display ID
#define ILI9341_RDDST       0x09 // Read Display Status
#define ILI9341_RDDPM       0x0A // Read Display Power
#define ILI9341_RDDMADCTL   0x0B // Read Display Memory Data Access Mode
#define ILI9341_RDDCOLMOD   0x0C // Read Display Pixel
#define ILI9341_RDDIM       0x0D // Read Display Image
#define ILI9341_RDDSM       0x0E // Read Display Signal
#define ILI9341_RDDSDR      0x0F // Read Display Self Diagnostics
#define ILI9341_SLPIN       0x10 // Sleep In
#define ILI9341_SLPOUT      0x11 // Sleep Out
#define ILI9341_PTLON       0x12 // Partial Mode On
#define ILI9341_NORON       0x13 // Partial Mode Off
#define ILI9341_INVOFF      0x20 // Display Invert Off
#define ILI9341_INVON       0x21 // Display Invert On
#define ILI9341_GAMSET      0x26 // Display Invert On Gamma
#define ILI9341_DISPOFF     0x28 // Display Off
#define ILI9341_DISPON      0x29 // Display On
#define ILI9341_CASET       0x2A // Column Address Set
#define ILI9341_RASET       0x2B // Row Address Set
#define ILI9341_RAMWR       0x2C // Memory Write
#define ILI9341_RAMRD       0x2E // Memory Read
#define ILI9341_PTLAR       0x30 // Partial Start/End Address Set
#define ILI9341_VSCRDEF     0x33 // Vertical Scrolling Definition
#define ILI9341_TEOFF       0x34 // Tearing Effect Line Off
#define ILI9341_TEON        0x35 // Tearing Effect Line On
#define ILI9341_MADCTL      0x36 // Memory Data Access Control
#define ILI9341_VSCRSADD    0x37 // Vertical Scrolling Start Address
#define ILI9341_IDMOFF      0x38 // Idle Mode Off
#define ILI9341_IDMON       0x39 // Idle Mode On
#define ILI9341_COLMOD      0x3A // Interface Pixel Format
#define ILI9341_RAMWRC      0x3C // Memory Write Continue
#define ILI9341_RAMRDC      0x3E // Memory Read Continue
#define ILI9341_TESCAN      0x44 // Set Tear Scan Line
#define ILI9341_RDTESCAN    0x45 // Get Tear Scan Line
#define ILI9341_WRDISBV     0x51 // Set Display Brightness
#define ILI9341_RDDISBV     0x52 // Get Display Brightness
#define ILI9341_WRCTRLD     0x53 // Set Display Control
#define ILI9341_RDCTRLD     0x54 // Get Display Control
#define ILI9341_WRCACE      0x55 // Write content adaptive brightness control and Color enhancement
#define ILI9341_RDCABC      0x56 // Read content adaptive brightness control and Color enhancement
#define ILI9341_WRCABCMB    0x5E // Write CABC minimum brightness
#define ILI9341_RDCABCMB    0x5F // Read CABC minimum brightness
#define ILI9341_RDABCSDR    0x68 // Read Automatic Brightness Control Self-Diagnostic Result
#define ILI9341_PORCTRK     0xB2 // Porch setting
#define ILI9341_GCTRL       0xB7 // Gate Control
#define ILI9341_VCOMS       0xBB // VCOM setting
#define ILI9341_LCMCTRL     0xC0 // LCM Control
#define ILI9341_VDVVRHEN    0xC2 // VDV and VRH Command Enable
#define ILI9341_VRHS        0xC3 // VRH Set
#define ILI9341_VDVS        0xC4 // VDV Set
#define ILI9341_FRCTRL2     0xC6 // Frame Rate control in normal mode
#define ILI9341_PWCTRL1     0xD0 // Power Control 1
#define ILI9341_RDID1       0xDA // Read ID1
#define ILI9341_RDID2       0xDB // Read ID2
#define ILI9341_RDID3       0xDC // Read ID3
#define ILI9341_PVGAMCTRL   0xE0 // Positive Voltage Gamma control
#define ILI9341_NVGAMCTRL   0xE1 // Negative Voltage Gamma control

__BEGIN_DECLS

extern esp_err_t driver_ili9341_init(void);
extern esp_err_t driver_ili9341_set_backlight(bool state);
extern esp_err_t driver_ili9341_set_sleep(bool state);
extern esp_err_t driver_ili9341_set_display(bool state);
extern esp_err_t driver_ili9341_set_invert(bool state);
extern esp_err_t driver_ili9341_write(const uint8_t *data);
extern esp_err_t driver_ili9341_write_partial(const uint8_t *buffer, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

__END_DECLS

#endif // DRIVER_ILI9341_H
