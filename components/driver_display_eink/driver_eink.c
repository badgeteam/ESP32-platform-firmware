#include <sdkconfig.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <esp_heap_caps.h>
#include <esp_log.h>

#include <nvs_flash.h>
#include <nvs.h>

#include "include/driver_eink_dev.h"
#include "include/driver_eink_lut.h"
#include "include/driver_eink.h"

#ifdef CONFIG_DRIVER_EINK_ENABLE

static const char *TAG = "e-ink";

static const uint8_t xlat_curve[256] = {
    0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x03,0x04,0x04,0x05,0x05,
    0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x09,0x0a,0x0a,0x0a,0x0b,
    0x0b,0x0c,0x0c,0x0d,0x0d,0x0e,0x0e,0x0f,0x0f,0x10,0x10,0x11,
    0x11,0x12,0x12,0x13,0x13,0x14,0x15,0x15,0x16,0x16,0x17,0x17,
    0x18,0x18,0x19,0x19,0x1a,0x1a,0x1b,0x1b,0x1c,0x1d,0x1d,0x1e,
    0x1e,0x1f,0x1f,0x20,0x20,0x21,0x22,0x22,0x23,0x23,0x24,0x25,
    0x25,0x26,0x26,0x27,0x27,0x28,0x29,0x29,0x2a,0x2a,0x2b,0x2c,
    0x2c,0x2d,0x2e,0x2e,0x2f,0x2f,0x30,0x31,0x31,0x32,0x33,0x33,
    0x34,0x35,0x35,0x36,0x37,0x37,0x38,0x39,0x39,0x3a,0x3b,0x3b,
    0x3c,0x3d,0x3e,0x3e,0x3f,0x40,0x40,0x41,0x42,0x43,0x43,0x44,
    0x45,0x46,0x46,0x47,0x48,0x49,0x49,0x4a,0x4b,0x4c,0x4c,0x4d,
    0x4e,0x4f,0x50,0x50,0x51,0x52,0x53,0x54,0x55,0x55,0x56,0x57,
    0x58,0x59,0x5a,0x5b,0x5b,0x5c,0x5d,0x5e,0x5f,0x60,0x61,0x62,
    0x63,0x64,0x65,0x66,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,
    0x6e,0x6f,0x70,0x71,0x72,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,
    0x7b,0x7c,0x7d,0x7e,0x80,0x81,0x82,0x83,0x84,0x86,0x87,0x88,
    0x89,0x8a,0x8c,0x8d,0x8e,0x90,0x91,0x92,0x93,0x95,0x96,0x98,
    0x99,0x9a,0x9c,0x9d,0x9f,0xa0,0xa2,0xa3,0xa5,0xa6,0xa8,0xa9,
    0xab,0xac,0xae,0xb0,0xb1,0xb3,0xb5,0xb6,0xb8,0xba,0xbc,0xbe,
    0xbf,0xc1,0xc3,0xc5,0xc7,0xc9,0xcb,0xcd,0xcf,0xd1,0xd3,0xd6,
    0xd8,0xda,0xdc,0xdf,0xe1,0xe3,0xe6,0xe8,0xeb,0xed,0xf0,0xf3,
    0xf5,0xf8,0xfb,0xfe,
};

static uint32_t *driver_eink_tmpbuf = NULL;
static uint32_t *driver_eink_oldbuf = NULL;
static bool driver_eink_have_oldbuf = false;

static void memcpy_u32(uint32_t *dst, const uint32_t *src, size_t size)
{
	while (size-- > 0) {
		*dst++ = *src++;
	}
}

static void memset_u32(uint32_t *dst, uint32_t value, size_t size)
{
	while (size-- > 0) {
		*dst++ = value;
	}
}

static void driver_eink_create_bitplane(const uint8_t *img, uint32_t *buf, int bit, driver_eink_flags_t flags)
{
#ifdef CONFIG_EPD_ROTATED_180
	flags ^= DISPLAY_FLAG_ROTATE_180;
#endif
	int x, y;
	int pos, dx, dy;
	if (flags & DISPLAY_FLAG_ROTATE_180)
	{
		pos = DISP_SIZE_Y-1;
		dx = DISP_SIZE_Y;
		dy = -DISP_SIZE_Y*DISP_SIZE_X - 1;
	}
	else
	{
		pos = (DISP_SIZE_X-1)*DISP_SIZE_Y;
		dx = -DISP_SIZE_Y;
		dy = DISP_SIZE_Y*DISP_SIZE_X + 1;
	}
	for (y = 0; y < DISP_SIZE_Y; y++) {
		for (x = 0; x < DISP_SIZE_X;) {
			int x_bits;
			uint32_t res = 0;
			for (x_bits=0; x_bits<32; x_bits++)
			{
				res <<= 1;
				if (flags & DISPLAY_FLAG_8BITPIXEL)
				{
					uint8_t pixel = img[pos];
					pos += dx;
					int j = xlat_curve[pixel];
					if ((j & bit) != 0)
						res++;
				}
				else
				{
					uint8_t pixel = img[pos >> 3] >> (pos & 7);
					pos += dx;
					if ((pixel & 1) != 0)
						res++;
				}
				x++;
			}
			*buf++ = res;
		}
		pos += dy;
	}
}

static void driver_eink_set_ram_area(uint8_t x_start, uint8_t x_end, uint16_t y_start, uint16_t y_end)
{
	// set RAM X - address Start / End position
	driver_eink_dev_write_command_p2(0x44, x_start, x_end);
	// set RAM Y - address Start / End position
	driver_eink_dev_write_command_p4(0x45, y_start & 0xff, y_start >> 8, y_end & 0xff, y_end >> 8);
}

static void driver_eink_set_ram_pointer(uint8_t x_addr, uint16_t y_addr)
{
	// set RAM X address counter
	driver_eink_dev_write_command_p1(0x4e, x_addr);
	// set RAM Y address counter
	driver_eink_dev_write_command_p2(0x4f, y_addr & 0xff, y_addr >> 8);
}

static void driver_eink_write_bitplane(const uint32_t *buf)
{
	driver_eink_set_ram_area(0, DISP_SIZE_X_B - 1, 0, DISP_SIZE_Y - 1);
	driver_eink_set_ram_pointer(0, 0);
	driver_eink_dev_write_command_stream_u32(0x24, buf, DISP_SIZE_X_B * DISP_SIZE_Y/4);
}

void driver_eink_update(const uint32_t *buf, const struct driver_eink_update *upd_conf)
{
	// generate lut data
	const struct driver_eink_lut_entry *lut_entries;

	if (upd_conf->lut == DRIVER_EINK_LUT_CUSTOM)
	{
		lut_entries = upd_conf->lut_custom;
	}
	else if (upd_conf->lut >= 0 && upd_conf->lut <= DRIVER_EINK_LUT_MAX)
	{
		const struct driver_eink_lut_entry *lut_lookup[DRIVER_EINK_LUT_MAX + 1] = {
			driver_eink_lut_full,
			driver_eink_lut_normal,
			driver_eink_lut_faster,
			driver_eink_lut_fastest,
		};
		lut_entries = lut_lookup[upd_conf->lut];
	}
	else
	{
		lut_entries = driver_eink_lut_full;
	}

	uint8_t lut[DRIVER_EINK_LUT_MAX_SIZE];
	int lut_len = driver_eink_lut_generate(lut_entries, upd_conf->lut_flags, lut);
	assert( lut_len >= 0 );

	driver_eink_dev_write_command_stream(0x32, lut, lut_len);

	if (buf == NULL)
		buf = driver_eink_tmpbuf;

	driver_eink_write_bitplane(buf);

	if (driver_eink_dev_type == DRIVER_EINK_DEPG0290B1 && driver_eink_have_oldbuf)
		driver_eink_dev_write_command_stream_u32(0x26, driver_eink_oldbuf, DISP_SIZE_X_B * DISP_SIZE_Y/4);

	// write number of overscan lines
	driver_eink_dev_write_command_p1(0x3a, upd_conf->reg_0x3a);

	// write time to write every line
	driver_eink_dev_write_command_p1(0x3b, upd_conf->reg_0x3b);

	uint16_t y_len = upd_conf->y_end - upd_conf->y_start;
	
	// configure length of update
	driver_eink_dev_write_command_p3(0x01, y_len & 0xff, y_len >> 8, 0x00);

	// configure starting-line of update
	driver_eink_dev_write_command_p2(0x0f, upd_conf->y_start & 0xff, upd_conf->y_start >> 8);

	// bitmapped enabled phases of the update: (in this order)
	//   80 - enable clock signal
	//   40 - enable CP
	//   20 - load temperature value
	//   10 - load LUT
	//   08 - initial display
	//   04 - pattern display
	//   02 - disable CP
	//   01 - disable clock signal
	driver_eink_dev_write_command_p1(0x22, 0xc7);

	// start update
	driver_eink_dev_write_command(0x20);

	if (driver_eink_dev_type == DRIVER_EINK_DEPG0290B1)
	{
		memcpy_u32(driver_eink_oldbuf, buf, DISP_SIZE_X_B * DISP_SIZE_Y/4);
	}
	driver_eink_have_oldbuf = true;
}

uint16_t minimal_update_height = DISP_SIZE_Y;

void driver_eink_set_minimal_update_height(uint16_t height)
{
	if (height > DISP_SIZE_Y) height = DISP_SIZE_Y;
	minimal_update_height = height;
}

void driver_eink_display_part(const uint8_t *img, driver_eink_flags_t flags, uint16_t y_start, uint16_t y_end)
{
	int lut_mode = (flags >> DISPLAY_FLAG_LUT_BIT) & ((1 << DISPLAY_FLAG_LUT_SIZE)-1);

	uint32_t *buf = driver_eink_tmpbuf;
	if (img == NULL) {
		memset_u32(buf, 0, DISP_SIZE_X_B * DISP_SIZE_Y/4);
	} else {
		driver_eink_create_bitplane(img, buf, 0x80, flags);
	}

	if ((flags & DISPLAY_FLAG_NO_UPDATE) != 0) {
		return;
	}

	int lut_flags = 0;
	if (!driver_eink_have_oldbuf || (flags & DISPLAY_FLAG_FULL_UPDATE)) {
		// old image not known (or full update requested); do full update
		lut_flags |= LUT_FLAG_FIRST;
	} else if (lut_mode - 1 != DRIVER_EINK_LUT_FULL) {
		// old image is known; prefer to do a partial update
		lut_flags |= LUT_FLAG_PARTIAL;
	}
	
	/*
	This piece of code makes the height of the updated area at least a certain
	size wide. This is done to make sure the pixels involved change enough to be
	noticable and it is a big workaround. The default minimum area is the height
	of the display, resulting in full updates.
	 */
	
	if (y_end-y_start+1 < minimal_update_height) {
		y_end += minimal_update_height+y_start-y_end;
		if (y_end >= DISP_SIZE_Y) {
			y_start -= y_end % DISP_SIZE_Y;
			y_end -= y_end % DISP_SIZE_Y;
		}
	}
	
	struct driver_eink_update eink_upd = {
		.lut       = lut_mode > 0 ? lut_mode - 1 : DRIVER_EINK_LUT_DEFAULT,
		.lut_flags = lut_flags,
		.reg_0x3a  = 26,   // 26 dummy lines per gate
		.reg_0x3b  = 0x08, // 62us per line
		.y_start   = y_start,
		.y_end     = y_end,
	};
	driver_eink_update(buf, &eink_upd);
}

void driver_eink_display(const uint8_t *img, driver_eink_flags_t flags)
{
	driver_eink_display_part(img, flags, 0, 295);
}

void driver_eink_display_greyscale(const uint8_t *img, driver_eink_flags_t flags, int layers)
{
	// start with black.
	driver_eink_display(NULL, flags | DISPLAY_FLAG_FULL_UPDATE);

	// the max. number of layers. more layers will result in more ghosting
	if (driver_eink_dev_type == DRIVER_EINK_DEPG0290B1 && layers > 5) {
		layers = 5;
	} else if (driver_eink_dev_type == DRIVER_EINK_GDEH029A1 && layers > 7) {
		layers = 7;
	}

	int p_ini = (driver_eink_dev_type == DRIVER_EINK_DEPG0290B1) ? 4 : 16;

	driver_eink_have_oldbuf = false;

	for (int layer = 0; layer < layers; layer++) {
		int bit = 128 >> layer;
		int t = bit;
		// gdeh: 128, 64, 32, 16, 8, 4, 2
		// depg: 128, 64, 32, 16, 8

		int p = p_ini;

		while ((t & 1) == 0 && (p > 1)) {
			t >>= 1;
			p >>= 1;
		}

		if (driver_eink_dev_type == DRIVER_EINK_DEPG0290B1 && driver_eink_have_oldbuf == false && p == 1 && t > 1 && layer+1 < layers) {
			driver_eink_create_bitplane(img, driver_eink_oldbuf, bit, flags);
			driver_eink_have_oldbuf = true;
			continue;
		}

		for (int j = 0; j < p; j++) {
			int y_start = 0 + j * (DISP_SIZE_Y / p);
			int y_end = y_start + (DISP_SIZE_Y / p) - 1;

			uint32_t *buf = driver_eink_tmpbuf;
			driver_eink_create_bitplane(img, buf, bit, flags);

			// clear borders
			memset_u32(buf, 0, y_start * DISP_SIZE_X_B/4);
			memset_u32(&buf[(y_end+1) * DISP_SIZE_X_B/4], 0, (DISP_SIZE_Y-y_end-1) * DISP_SIZE_X_B/4);

			struct driver_eink_lut_entry lut[4];

			if (driver_eink_have_oldbuf) {
				// LUT:
				//   Use old state as previous layer;
				//   Do nothing when bits are not set;
				//   Make pixel whiter when bit is set;
				//   Duration is <t> cycles.
				lut[0].length = t;
				lut[0].voltages = 0x80;
				lut[1].length = t;
				lut[1].voltages = 0xa0;
				lut[2].length = t;
				lut[2].voltages = 0xa8;
				lut[3].length = 0;

			} else {
				// LUT:
				//   Ignore old state;
				//   Do nothing when bit is not set;
				//   Make pixel whiter when bit is set;
				//   Duration is <t> cycles.
				lut[0].length = t;
				lut[0].voltages = 0x88;
				lut[1].length = 0;
			}

			/* update display */
			struct driver_eink_update eink_upd = {
				.lut        = DRIVER_EINK_LUT_CUSTOM,
				.lut_custom = lut,
				.reg_0x3a   = 0, // no dummy lines per gate
				.reg_0x3b   = 0, // 30us per line
				.y_start    = y_start,
				.y_end      = y_end + 1,
			};
			driver_eink_update(buf, &eink_upd);
			driver_eink_have_oldbuf = false;
		}
	}
}

void driver_eink_deep_sleep(void)
{
	driver_eink_dev_write_command_p1(0x10, 0x01); // enter deep sleep
}

void driver_eink_wakeup(void)
{
	driver_eink_dev_write_command_p1(0x10, 0x00); // leave deep sleep
}

esp_err_t driver_eink_init(void)
{
	static bool driver_eink_init_done = false;
	
	if (driver_eink_init_done) return ESP_OK;

	ESP_LOGD(TAG, "init called");
	
	uint8_t eink_type;
	nvs_handle my_handle;
	esp_err_t err = nvs_open("system", NVS_READWRITE, &my_handle);
	if (err != ESP_OK) { ESP_LOGE(TAG, "nvs handle failure"); return err; }
	err = nvs_get_u8(my_handle, "eink.dev.type", &eink_type);
	if (err != ESP_OK) {
		//Failed to get e-ink type from NVS, write value from menuconfig
		eink_type = DRIVER_EINK_DEFAULT;
		nvs_set_u8(my_handle, "eink.dev.type", eink_type);
	}
	
	//printf("E-ink type is %d\n", eink_type);
		
	enum driver_eink_dev_t dev_type = eink_type;

	// initialize spi interface to display
	esp_err_t res = driver_eink_dev_init(dev_type);
	if (res != ESP_OK) { ESP_LOGE(TAG, "dev init failure");  return res; }

	// allocate buffers
	driver_eink_tmpbuf = heap_caps_malloc(DISP_SIZE_X_B * DISP_SIZE_Y, MALLOC_CAP_32BIT);
	if (driver_eink_tmpbuf == NULL) { ESP_LOGE(TAG, "tmpbuf alloc no mem"); return ESP_ERR_NO_MEM; }

	if (driver_eink_dev_type == DRIVER_EINK_DEPG0290B1) {
		driver_eink_oldbuf = heap_caps_malloc(DISP_SIZE_X_B * DISP_SIZE_Y, MALLOC_CAP_32BIT);
		if (driver_eink_oldbuf == NULL) { ESP_LOGE(TAG, "oldbuf alloc no mem");  return ESP_ERR_NO_MEM; }
	}

	if (driver_eink_dev_type == DRIVER_EINK_GDEH029A1) {
		/* initialize GDEH029A1 */
		//printf("E-ink init GDEH029A1\n");
		driver_eink_dev_reset(); // Hardware reset
		driver_eink_dev_write_command(0x12); // Software reset
		driver_eink_dev_write_command_p3(0x0c, 0xd7, 0xd6, 0x9d); // 0C: booster soft start control
		driver_eink_dev_write_command_p1(0x2c, 0xa8); // 2C: write VCOM register (VCOM 7c)
		driver_eink_dev_write_command_p1(0x11, 0x03); // 11: data entry mode setting (X inc, Y inc)
	}

	if (driver_eink_dev_type == DRIVER_EINK_DEPG0290B1)
	{
		/* initialize DEPG0290B01 */
		//printf("E-ink init DEPG0290B01\n");
		driver_eink_dev_reset(); // Hardware reset
		driver_eink_dev_write_command(0x12); // Software reset
		driver_eink_dev_write_command_p1(0x74, 0x54); // Set analog block control
		driver_eink_dev_write_command_p1(0x7E, 0x3B); // Set digital block control
		driver_eink_dev_write_command_p3(0x01, 0x27, 0x01, 0x00); // Set display size and driver output control
		driver_eink_dev_write_command_p1(0x11, 0x03); // Ram data entry mode (Adress counter is updated in Y direction, Y increment, X increment)
		driver_eink_dev_write_command_p2(0x44, 0x00, 0x0F); // Set RAM X address (00h to 0Fh)
		driver_eink_dev_write_command_p4(0x45, 0x00, 0x00, 0x27, 0x01); // Set RAM Y address (0127h to 0000h)
		driver_eink_dev_write_command_p1(0x3C, 0x01); // Set border waveform for VBD (see datasheet)
		driver_eink_dev_write_command_p1(0x2C, 0x26); // Set VCOM value (SET VOLTAGE)
		driver_eink_dev_write_command_p1(0x03, 0x17); // Gate voltage setting (17h = 20 Volt, ranges from 10v to 21v) (SET VOLTAGE)
		driver_eink_dev_write_command_p3(0x04, 0x41, 0x00, 0x32); // Source voltage setting (15volt, 0 volt and -15 volt) (SET VOLTAGE)
	}

	driver_eink_init_done = true;

	ESP_LOGD(TAG, "init done");

	return ESP_OK;
}
#else
esp_err_t driver_eink_init(void) { return ESP_OK; } //Dummy function.
#endif // CONFIG_DRIVER_EINK_ENABLE
