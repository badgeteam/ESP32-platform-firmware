#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "sdkconfig.h"
#include "apps/sntp/sntp.h"
#include "driver/rtc_io.h"
#include "esp_log.h"
#include "rom/crc.h"

#include <esp_log.h>
#include <esp_err.h>

#include "include/driver_rtcmem.h"

#define TAG "rtcmem"

#define RTC_MEM_INT_SIZE 64
#define RTC_MEM_STR_SIZE 2048

static int      RTC_DATA_ATTR rtc_mem_int[RTC_MEM_INT_SIZE] = { 0 };
static uint16_t RTC_DATA_ATTR rtc_mem_int_crc;

static char     RTC_DATA_ATTR rtc_mem_str[RTC_MEM_STR_SIZE] = { 0 };
static uint16_t RTC_DATA_ATTR rtc_mem_str_crc;

esp_err_t driver_rtcmem_int_write(int pos, int val)
{
	if (pos >= RTC_MEM_INT_SIZE) return ESP_FAIL;
	rtc_mem_int[pos] = val;
	rtc_mem_int_crc = crc16_le(0, (uint8_t const *)rtc_mem_int, RTC_MEM_INT_SIZE*sizeof(int));
	return ESP_OK;
}

esp_err_t driver_rtcmem_int_read(int pos, int* val)
{
	if (pos >= RTC_MEM_INT_SIZE) return ESP_FAIL;
	if (rtc_mem_int_crc != crc16_le(0, (uint8_t const *)rtc_mem_int, RTC_MEM_INT_SIZE*sizeof(int))) return ESP_FAIL;
	*val = rtc_mem_int[pos];
	return ESP_OK;
}

esp_err_t driver_rtcmem_string_write(const char* str)
{
	if (strlen(str) >= RTC_MEM_STR_SIZE) return ESP_FAIL;
	memset(rtc_mem_str, 0, sizeof(rtc_mem_str));
	strcpy(rtc_mem_str, str);
	rtc_mem_str_crc = crc16_le(0, (uint8_t const *)rtc_mem_str, RTC_MEM_STR_SIZE);
	return ESP_OK;
}

esp_err_t driver_rtcmem_string_read(const char** str)
{
	if (rtc_mem_str_crc != crc16_le(0, (uint8_t const *)rtc_mem_str, RTC_MEM_STR_SIZE)) return ESP_FAIL;
	*str = rtc_mem_str;
	return ESP_OK;
}

esp_err_t driver_rtcmem_clear()
{
	memset(rtc_mem_int, 0, sizeof(rtc_mem_int));
	memset(rtc_mem_str, 0, sizeof(rtc_mem_str));
	rtc_mem_int_crc = 0;
	rtc_mem_str_crc = 0;
	return ESP_OK;
}

esp_err_t driver_rtcmem_init(void)
{
	//Empty
	return ESP_OK;
}
