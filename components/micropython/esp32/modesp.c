/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * Development of the code in this file was sponsored by Microbric Pty Ltd
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Paul Sokolovsky
 * Copyright (c) 2016 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include "esp_spi_flash.h"
#include "wear_levelling.h"
#include "modesp.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "py/runtime.h"
#include "rom/rtc.h"

#include <esp_heap_caps.h>

#if MICROPY_SDMMC_USE_DRIVER
#include "badge_power.h"
#include "badge_sdcard.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"

#include "driver_rtcmem.h"

#define TAG "modesp"

STATIC mp_obj_t badge_raminfo_() {
    size_t free_8           = heap_caps_get_free_size(MALLOC_CAP_8BIT  | MALLOC_CAP_INTERNAL);
    size_t free_32          = heap_caps_get_free_size(MALLOC_CAP_32BIT | MALLOC_CAP_INTERNAL);
    size_t free_external    = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);

    size_t largest_8        = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT  | MALLOC_CAP_INTERNAL);
    size_t largest_32       = heap_caps_get_largest_free_block(MALLOC_CAP_32BIT | MALLOC_CAP_INTERNAL);
    size_t largest_external = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);

    printf("\nFree memory\n---------------------------------------------\n");
    printf("8  bit accessible:  %d\n", free_8);
    printf("32 bit accessible:  %d\n", free_32);
    printf("External (SPI) RAM: %d\n", free_external);
    printf("IRAM (32 - 8):      %d\n", free_32-free_8);

    printf("\nLargest available region\n---------------------------------------------\n");
    printf("8  bit accessible:  %d\n", largest_8);
    printf("32 bit accessible:  %d\n", largest_32);
    printf("External (SPI) RAM: %d\n", largest_external);
    printf("IRAM (32 - 8):      %d\n", largest_32-largest_8);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(badge_raminfo_obj, badge_raminfo_);

STATIC mp_obj_t badge_ramdump_() {
    heap_caps_dump_all();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(badge_ramdump_obj, badge_ramdump_);

/* esp.temperature_sens_read() */
extern int temprature_sens_read();
STATIC mp_obj_t esp_temperature_sens_read() {
  return mp_obj_new_int_from_uint(temprature_sens_read());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(esp_temperature_sens_read_obj, esp_temperature_sens_read);

/* esp.hall_sens_read() */
extern int hall_sens_read();
STATIC mp_obj_t esp_hall_sens_read() {
  return mp_obj_new_int_from_uint(hall_sens_read());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(esp_hall_sens_read_obj, esp_hall_sens_read);


/* esp.wdt_start(<secs>) */
STATIC mp_obj_t esp_wdt_start(mp_obj_t timeout) {
	// reconfigure watchdog
	mp_int_t t = mp_obj_get_int(timeout);
	esp_err_t res = esp_task_wdt_init(t, true);
	if (res != ESP_OK) {
		if (res == ESP_ERR_NO_MEM)
			mp_raise_msg(&mp_type_MemoryError, "WDT: Out of memory");
		mp_raise_msg(&mp_type_NotImplementedError, "WDT: Unknown error");
	}

	// add current task
	res = esp_task_wdt_add(NULL);
	if (res != ESP_OK) {
		if (res == ESP_ERR_INVALID_ARG)
			mp_raise_msg(&mp_type_AttributeError, "WDT: Task is already subscribed");
		if (res == ESP_ERR_NO_MEM)
			mp_raise_msg(&mp_type_MemoryError, "WDT: Out of memory");
		if (res == ESP_ERR_INVALID_STATE)
			mp_raise_msg(&mp_type_AttributeError, "WDT: Not initialized");
		mp_raise_msg(&mp_type_NotImplementedError, "WDT: Unknown error");
	}

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(esp_wdt_start_obj, esp_wdt_start);

/* esp.wdt_stop() */
STATIC mp_obj_t esp_wdt_stop(void) {
	// remove current task
	esp_err_t res = esp_task_wdt_delete(NULL);
	if (res != ESP_OK) {
		if (res == ESP_ERR_INVALID_ARG)
			mp_raise_msg(&mp_type_AttributeError, "WDT: Task is already unsubscribed");
		if (res == ESP_ERR_INVALID_STATE)
			mp_raise_msg(&mp_type_AttributeError, "WDT: Not initialized");
		mp_raise_msg(&mp_type_NotImplementedError, "WDT: Unknown error");
	}

	// reconfigure watchdog to startup-state.
#ifdef CONFIG_TASK_WDT_PANIC
	res = esp_task_wdt_init(CONFIG_TASK_WDT_TIMEOUT_S, true);
#elif CONFIG_TASK_WDT
	res = esp_task_wdt_init(CONFIG_TASK_WDT_TIMEOUT_S, false);
#endif
	if (res != ESP_OK) {
		if (res == ESP_ERR_NO_MEM)
			mp_raise_msg(&mp_type_MemoryError, "WDT: Out of memory");
		mp_raise_msg(&mp_type_NotImplementedError, "WDT: Unknown error");
	}

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(esp_wdt_stop_obj, esp_wdt_stop);

/* esp.wdt_reset() */
STATIC mp_obj_t esp_wdt_reset(void) {
	esp_err_t res = esp_task_wdt_reset();
	if (res != ESP_OK) {
		if (res == ESP_ERR_NOT_FOUND)
			mp_raise_msg(&mp_type_AttributeError, "WDT: Task is not subscribed");
		if (res == ESP_ERR_INVALID_STATE)
			mp_raise_msg(&mp_type_AttributeError, "WDT: Not initialized");
		mp_raise_msg(&mp_type_NotImplementedError, "WDT: Unknown error");
	}

	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(esp_wdt_reset_obj, esp_wdt_reset);


/* flash */
STATIC wl_handle_t fs_handle = WL_INVALID_HANDLE;
STATIC size_t wl_sect_size = 4096;

STATIC esp_partition_t fs_part;

STATIC mp_obj_t esp_flash_read(mp_obj_t offset_in, mp_obj_t buf_in) {
    mp_int_t offset = mp_obj_get_int(offset_in);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_WRITE);

    esp_err_t res = wl_read(fs_handle, offset, bufinfo.buf, bufinfo.len);
    if (res != ESP_OK) {
        mp_raise_OSError(MP_EIO);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(esp_flash_read_obj, esp_flash_read);

STATIC mp_obj_t esp_flash_write(mp_obj_t offset_in, mp_obj_t buf_in) {
    mp_int_t offset = mp_obj_get_int(offset_in);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_READ);

    esp_err_t res = wl_write(fs_handle, offset, bufinfo.buf, bufinfo.len);
    if (res != ESP_OK) {
        mp_raise_OSError(MP_EIO);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(esp_flash_write_obj, esp_flash_write);

STATIC mp_obj_t esp_flash_erase(mp_obj_t sector_in) {
    mp_int_t sector = mp_obj_get_int(sector_in);

    esp_err_t res = wl_erase_range(fs_handle, sector * wl_sect_size, wl_sect_size);
    if (res != ESP_OK) {
        mp_raise_OSError(MP_EIO);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(esp_flash_erase_obj, esp_flash_erase);

STATIC mp_obj_t esp_flash_size(void) {
  if (fs_handle == WL_INVALID_HANDLE) {
      const esp_partition_t *part
          = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "locfd");
      if (part == NULL) {
        printf("No part\n");
          return mp_obj_new_int_from_uint(0);
      }
      memcpy(&fs_part, part, sizeof(esp_partition_t));

      esp_err_t res = wl_mount(&fs_part, &fs_handle);
      if (res != ESP_OK) {
        printf("No mount\n");
          return mp_obj_new_int_from_uint(0);
      }
      wl_sect_size = wl_sector_size(fs_handle);
  }
  return mp_obj_new_int_from_uint(fs_part.size);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(esp_flash_size_obj, esp_flash_size);

STATIC mp_obj_t esp_flash_sec_size() {
  return mp_obj_new_int_from_uint(wl_sect_size);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(esp_flash_sec_size_obj, esp_flash_sec_size);

STATIC IRAM_ATTR mp_obj_t esp_flash_user_start(void) {
  return MP_OBJ_NEW_SMALL_INT(0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(esp_flash_user_start_obj, esp_flash_user_start);

STATIC mp_obj_t dump_mem_allocs(mp_obj_t cap) {
	uint32_t cap_id = mp_obj_get_int(cap);
	heap_caps_print_heap_info( cap_id );
	return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(dump_mem_allocs_obj, dump_mem_allocs);

STATIC mp_obj_t esp_rtc_get_reset_reason_(mp_obj_t cpu) {
  uint8_t cpu_id = mp_obj_get_int(cpu);
  if (cpu_id > 1) {
    return mp_obj_new_int(0);
  }
  uint8_t val = rtc_get_reset_reason(cpu_id);
  return mp_obj_new_int(val);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(esp_rtc_get_reset_reason_obj,
                                 esp_rtc_get_reset_reason_);

#if MICROPY_SDMMC_USE_DRIVER

// ======== SD Card support ===========================================================================

STATIC sdmmc_card_t sdmmc_card;
STATIC uint8_t sdcard_status = 1;

//---------------------------------------------------------------
STATIC void sdcard_print_info(const sdmmc_card_t* card, int mode)
{
    #if MICROPY_SDMMC_SHOW_INFO
    printf("---------------------\n");
    if (mode == 1) {
        printf(" Mode: 1-line mode\n");
    } else {
        printf(" Mode:  SD (4bit)\n");
    }
    printf(" Name: %s\n", card->cid.name);
    printf(" Type: %s\n", (card->ocr & SD_OCR_SDHC_CAP)?"SDHC/SDXC":"SDSC");
    printf("Speed: %s (%d MHz)\n", (card->csd.tr_speed > 25000000)?"high speed":"default speed", card->csd.tr_speed/1000000);
    printf(" Size: %u MB\n", (uint32_t)(((uint64_t) card->csd.capacity) * card->csd.sector_size / (1024 * 1024)));
    printf("  CSD: ver=%d, sector_size=%d, capacity=%d read_bl_len=%d\n",
            card->csd.csd_ver,
            card->csd.sector_size, card->csd.capacity, card->csd.read_block_len);
    printf("  SCR: sd_spec=%d, bus_width=%d\n\n", card->scr.sd_spec, card->scr.bus_width);
    #endif
}

//----------------------------------------------
STATIC mp_obj_t esp_sdcard_init() {
    badge_power_sdcard_enable();
    badge_sdcard_init();

    mp_int_t card_mode = 1;

    if (sdcard_status == 0) {
        #if MICROPY_SDMMC_SHOW_INFO
        printf("Already initialized:\n");
        sdcard_print_info(&sdmmc_card, card_mode);
        #endif
        return MP_OBJ_NEW_SMALL_INT(sdcard_status);
    }

    // Configure sdmmc interface
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

	  if (card_mode == 1) {
        // Use 1-line SD mode
        host.flags = SDMMC_HOST_FLAG_1BIT;
        slot_config.width = 1;
    }

    sdmmc_host_init();
    sdmmc_host_init_slot(SDMMC_HOST_SLOT_1, &slot_config);

    // Initialize the sd card
    esp_log_level_set("*", ESP_LOG_NONE);
    #if MICROPY_SDMMC_SHOW_INFO
	  printf("---------------------\n");
    printf("Initializing SD Card: ");
    #endif
    esp_err_t res = sdmmc_card_init(&host, &sdmmc_card);
	  esp_log_level_set("*", ESP_LOG_ERROR);

    if (res == ESP_OK) {
        sdcard_status = ESP_OK;
        #if MICROPY_SDMMC_SHOW_INFO
        printf("OK.\n");
        #endif
        sdcard_print_info(&sdmmc_card, card_mode);
    } else {
        sdcard_status = 1;
        #if MICROPY_SDMMC_SHOW_INFO
        printf("Error.\n\n");
        #endif
    }
    return MP_OBJ_NEW_SMALL_INT(sdcard_status);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(esp_sdcard_init_obj, esp_sdcard_init);

//-------------------------------------------------------------------------
STATIC mp_obj_t esp_sdcard_read(mp_obj_t ulSectorNumber, mp_obj_t buf_in) {
    mp_int_t sect_num = mp_obj_get_int(ulSectorNumber);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_WRITE);
    mp_int_t sect_count = bufinfo.len / sdmmc_card.csd.sector_size;
    mp_int_t sect_err = bufinfo.len % sdmmc_card.csd.sector_size;

    if (sdcard_status != ESP_OK) {
        mp_raise_OSError(MP_EIO);
    }
    if (sect_count == 0) {
        mp_raise_OSError(MP_EIO);
    }
    if (sect_err) {
        mp_raise_OSError(MP_EIO);
    }

    esp_err_t res = sdmmc_read_sectors(&sdmmc_card, bufinfo.buf, sect_num, sect_count);
    if (res != ESP_OK) {
        mp_raise_OSError(MP_EIO);
    }

    //printf("[SD] read sect=%d, count=%d, size=%d\n", sect_num, sect_count, bufinfo.len);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(esp_sdcard_read_obj, esp_sdcard_read);


//--------------------------------------------------------------------------
STATIC mp_obj_t esp_sdcard_write(mp_obj_t ulSectorNumber, mp_obj_t buf_in) {
    mp_int_t sect_num = mp_obj_get_int(ulSectorNumber);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_READ);
    mp_int_t sect_count = bufinfo.len / sdmmc_card.csd.sector_size;
    mp_int_t sect_err = bufinfo.len % sdmmc_card.csd.sector_size;

    if (sdcard_status != ESP_OK) {
        mp_raise_OSError(MP_EIO);
    }
    if (sect_count == 0) {
        mp_raise_OSError(MP_EIO);
    }
    if (sect_err) {
        mp_raise_OSError(MP_EIO);
    }

    int res = sdmmc_write_sectors(&sdmmc_card, bufinfo.buf, sect_num, sect_count);
    if (res != ESP_OK) {
        mp_raise_OSError(MP_EIO);
    }

    //printf("[SD] write sect=%d, count=%d, size=%d\n", sect_num, sect_count, bufinfo.len);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(esp_sdcard_write_obj, esp_sdcard_write);


//-------------------------------------------
STATIC mp_obj_t esp_sdcard_sect_count(void) {
    if (sdcard_status == ESP_OK) {
        return MP_OBJ_NEW_SMALL_INT(sdmmc_card.csd.capacity);
    }
    else {
        return MP_OBJ_NEW_SMALL_INT(0);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(esp_sdcard_sect_count_obj, esp_sdcard_sect_count);

//------------------------------------------
STATIC mp_obj_t esp_sdcard_sect_size(void) {
    if (sdcard_status == ESP_OK) {
        return MP_OBJ_NEW_SMALL_INT(sdmmc_card.csd.sector_size);
    }
    else {
        return MP_OBJ_NEW_SMALL_INT(0);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(esp_sdcard_sect_size_obj, esp_sdcard_sect_size);


// ======== ^^^^^^^^^^^^^^^ ===========================================================================

#endif  // MICROPY_SDMMC_USE_DRIVER

// Partition table upgrade code for sha2017 and disobey 2019 badges

#ifdef CONFIG_FW_ENABLE_SHA2017_DISOBEY2019_PARTITION_TABLE_UPGRADE
#define PARTITIONS_16MB_BIN_LEN 192
#define PARTITIONS_LOCATION 0x8000
#define PARTITIONS_SECTOR PARTITIONS_LOCATION/SPI_FLASH_SEC_SIZE
static mp_obj_t esp_update_partition_table(void) {
    unsigned char check[PARTITIONS_16MB_BIN_LEN];
    // This is the new sha2017 16MB partition table
    unsigned char partitions_16MB_bin[PARTITIONS_16MB_BIN_LEN] = {
        0xaa, 0x50, 0x01, 0x02, 0x00, 0x90, 0x00, 0x00,
        0x00, 0x40, 0x00, 0x00, 0x6e, 0x76, 0x73, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xaa, 0x50, 0x01, 0x00, 0x00, 0xd0, 0x00, 0x00,
        0x00, 0x20, 0x00, 0x00, 0x6f, 0x74, 0x61, 0x64,
        0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xaa, 0x50, 0x01, 0x01, 0x00, 0xf0, 0x00, 0x00,
        0x00, 0x10, 0x00, 0x00, 0x70, 0x68, 0x79, 0x5f,
        0x69, 0x6e, 0x69, 0x74, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xaa, 0x50, 0x00, 0x10, 0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x18, 0x00, 0x6f, 0x74, 0x61, 0x5f,
        0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xaa, 0x50, 0x00, 0x11, 0x00, 0x00, 0x19, 0x00,
        0x00, 0x00, 0x18, 0x00, 0x6f, 0x74, 0x61, 0x5f,
        0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xaa, 0x50, 0x01, 0x81, 0x00, 0x00, 0x31, 0x00,
        0x00, 0x00, 0xcf, 0x00, 0x6c, 0x6f, 0x63, 0x66, 
        0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    esp_err_t err = spi_flash_read(PARTITIONS_LOCATION, check, PARTITIONS_16MB_BIN_LEN);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error while reading old partition table! (0x%02x)", err);
        return mp_obj_new_bool(false);
    }
    
    if (memcmp(check, partitions_16MB_bin, PARTITIONS_16MB_BIN_LEN)==0) {
        ESP_LOGW(TAG, "Partition table already up-to-date!");
        return mp_obj_new_bool(true);
    }
    
    err = spi_flash_erase_sector(PARTITIONS_SECTOR);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not erase the old partition table! (0x%02x)", err);
    } else {
        err = spi_flash_write(PARTITIONS_LOCATION, partitions_16MB_bin, PARTITIONS_16MB_BIN_LEN);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Could not write the partition table! (0x%02x)", err);
        }
    }
    
    err = spi_flash_read(PARTITIONS_LOCATION, check, PARTITIONS_16MB_BIN_LEN);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error while reading back the new partition table! (0x%02x)", err);
        return mp_obj_new_bool(false);
    }

    if (memcmp(check, partitions_16MB_bin, PARTITIONS_16MB_BIN_LEN)==0) {
        return mp_obj_new_bool(true);
    } else {
        ESP_LOGE(TAG, "Error while verifying new partition table!");
        for (uint16_t i = 0; i < PARTITIONS_16MB_BIN_LEN; i++) {
            if (check[i]!=partitions_16MB_bin[i]) {
                ESP_LOGE(TAG, "Expected %02X at %02X, read %02X", partitions_16MB_bin[i], i, check[i]);
            }
        }
    }
    return mp_obj_new_bool(false);
}

static MP_DEFINE_CONST_FUN_OBJ_0(esp_update_partition_table_obj, esp_update_partition_table);

#endif

STATIC const mp_rom_map_elem_t esp_module_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_esp)},
    
	{MP_ROM_QSTR(MP_QSTR_raminfo), MP_ROM_PTR(&badge_raminfo_obj)},
	//{MP_ROM_QSTR(MP_QSTR_ramdump), MP_ROM_PTR(&badge_ramdump_obj)},

    { MP_ROM_QSTR(MP_QSTR_temperature_sens_read), MP_ROM_PTR(&esp_temperature_sens_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_hall_sens_read), MP_ROM_PTR(&esp_hall_sens_read_obj) },

    { MP_ROM_QSTR(MP_QSTR_wdt_start), MP_ROM_PTR(&esp_wdt_start_obj) },
    { MP_ROM_QSTR(MP_QSTR_wdt_stop), MP_ROM_PTR(&esp_wdt_stop_obj) },
    { MP_ROM_QSTR(MP_QSTR_wdt_reset), MP_ROM_PTR(&esp_wdt_reset_obj) },

    /*{ MP_ROM_QSTR(MP_QSTR_flash_read), MP_ROM_PTR(&esp_flash_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_flash_write), MP_ROM_PTR(&esp_flash_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_flash_erase), MP_ROM_PTR(&esp_flash_erase_obj) },
    { MP_ROM_QSTR(MP_QSTR_flash_size), MP_ROM_PTR(&esp_flash_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_flash_user_start), MP_ROM_PTR(&esp_flash_user_start_obj) },
    { MP_ROM_QSTR(MP_QSTR_flash_sec_size), MP_ROM_PTR(&esp_flash_sec_size_obj) },*/

	{ MP_ROM_QSTR(MP_QSTR_dump_mem_allocs), MP_ROM_PTR(&dump_mem_allocs_obj) },

    {MP_ROM_QSTR(MP_QSTR_rtc_get_reset_reason),
     MP_ROM_PTR(&esp_rtc_get_reset_reason_obj)},

    /*#if MICROPY_SDMMC_USE_DRIVER
    { MP_ROM_QSTR(MP_QSTR_sdcard_read), MP_ROM_PTR(&esp_sdcard_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_sdcard_write), MP_ROM_PTR(&esp_sdcard_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_sdcard_init), MP_ROM_PTR(&esp_sdcard_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_sdcard_sect_count), MP_ROM_PTR(&esp_sdcard_sect_count_obj) },
    { MP_ROM_QSTR(MP_QSTR_sdcard_sect_size), MP_ROM_PTR(&esp_sdcard_sect_size_obj) },
    // // class constants
    // { MP_ROM_QSTR(MP_QSTR_SD_1LINE), MP_ROM_INT(1) },
    // { MP_ROM_QSTR(MP_QSTR_SD_4LINE), MP_ROM_INT(4) },
    #endif*/

    #ifdef CONFIG_FW_ENABLE_SHA2017_DISOBEY2019_PARTITION_TABLE_UPGRADE
    { MP_ROM_QSTR(MP_QSTR_update_partition_table), MP_ROM_PTR(&esp_update_partition_table_obj) },
    #endif
};

STATIC MP_DEFINE_CONST_DICT(esp_module_globals, esp_module_globals_table);

const mp_obj_module_t esp_module = {
    .base = {&mp_type_module}, .globals = (mp_obj_dict_t *)&esp_module_globals,
};
