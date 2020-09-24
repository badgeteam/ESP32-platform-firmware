#include <sdkconfig.h>

#include <string.h>
#include <fcntl.h>

#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_spi_flash.h>
#include <esp_partition.h>
#include <esp_vfs.h>
#include <esp_vfs_fat.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <nvs.h>
#include <wear_levelling.h>

#include <file_reader.h>
#include <flash_reader.h>
#include <deflate_reader.h>
#include <png_reader.h>


#define TAG "ZIP"

esp_err_t mount_locfd()
{
	wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
	
	const esp_vfs_fat_mount_config_t mount_config = {
		.max_files              = 8,
		.format_if_mount_failed = true,
	};
	
	esp_err_t res = esp_vfs_fat_spiflash_mount("", "locfd", &mount_config, &s_wl_handle);
	
	if (res != ESP_OK) {
		ESP_LOGE(TAG, "Failed to mount FATFS (0x%x)", res);
		return ESP_FAIL;
	}
	
	return ESP_OK;
}

esp_err_t unpack_first_boot_zip()
{
	printf("Mounting FAT filesystem...\n");
	if (mount_locfd() != ESP_OK) return ESP_FAIL;
	
	const esp_partition_t * part_ota1 = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, "ota_1");
	if (part_ota1 == NULL) {
		ESP_LOGE(TAG, "ota1 partition not found!");
		return ESP_FAIL;
	}
	
	printf("Partition OTA1 is at 0x%08X\n", part_ota1->address);
	
	struct lib_deflate_reader *dr = (struct lib_deflate_reader *) malloc(sizeof(struct lib_deflate_reader));
	if (dr == NULL) {
		ESP_LOGE(TAG, "failed to init deflate object");
		return ESP_ERR_NO_MEM;
	}

	struct lib_flash_reader *fr = lib_flash_new(part_ota1, 4096);
	if (fr == NULL) {
		ESP_LOGE(TAG, "failed to init flash-reader");
		return ESP_ERR_NO_MEM;
	}
	
	printf("Unpacking ZIP...\n");
	
	bool first_chunk = true;
	while (1) {
		uint32_t pk_sig;
		ssize_t res = lib_flash_read(fr, (uint8_t *) &pk_sig, 4);
		if (res == -1 || res != 4) {
			ESP_LOGE(TAG, "(%d) failed to read from flash", __LINE__);
			return ESP_FAIL;
		}
		if (pk_sig == 0x04034b50) {
			struct {
				uint16_t version_need;
				uint16_t gp_bit_flag;
				uint16_t compr_method;
				uint16_t file_time;
				uint16_t file_date;
				uint32_t crc32;
				uint32_t compr_size;
				uint32_t uncompr_size;
				uint16_t fname_len;
				uint16_t ext_len;
			} __attribute__((packed)) local_file_header;

			res = lib_flash_read(fr, (uint8_t *) &local_file_header, sizeof(local_file_header));
			if (res == -1 || res != sizeof(local_file_header)) {
				ESP_LOGE(TAG, "(%d) failed to read from flash", __LINE__);
				return ESP_FAIL;
			}

			char fname[256];
			if (local_file_header.fname_len == 0) {
				ESP_LOGE(TAG, "filename too short");
				return ESP_FAIL;
			}
			
			if (local_file_header.fname_len >= sizeof(fname)-1) {
				ESP_LOGE(TAG, "filename too long");
				return ESP_FAIL;
			}

			res = lib_flash_read(fr, (uint8_t *) &fname[1], local_file_header.fname_len);
			if (res == -1 || res != local_file_header.fname_len) {
				ESP_LOGE(TAG, "(%d) failed to read from flash", __LINE__);
				return ESP_FAIL;
			}
			fname[ 0 ] = '/';
			local_file_header.fname_len += 1;
			fname[ local_file_header.fname_len ] = 0;

			while (local_file_header.ext_len > 0) {
				uint8_t tmpbuf[16];
				int sz = local_file_header.ext_len > 16 ? 16 : local_file_header.ext_len;
				res = lib_flash_read(fr, tmpbuf, sz);
				if (res == -1 || res != sz)
				{
					ESP_LOGE(TAG, "(%d) failed to read from flash", __LINE__);
					return ESP_FAIL;
				}
				local_file_header.ext_len -= sz;
			}

			if (fname[ local_file_header.fname_len - 1 ] == '/') { // dir
				fname[ local_file_header.fname_len - 1 ] = 0;
				ESP_LOGE(TAG, "mkdir('%s')", fname);
				int err = mkdir(fname, 0755);
				if (err < 0)
				{
					ESP_LOGE(TAG, "failed to create dir '%s': %d", fname, errno);
					return ESP_FAIL;
				}
			} else { // file
				lib_reader_read_t reader = (lib_reader_read_t) &lib_flash_read;
				void *reader_obj = fr;

				if (local_file_header.compr_method == 8) { // deflated
					lib_deflate_init(dr, reader, reader_obj);

					reader = (lib_reader_read_t) &lib_deflate_read;
					reader_obj = dr;
				} else if (local_file_header.compr_method != 0) { // not stored
					ESP_LOGE(TAG, "unknown compression type for file '%s'", fname);
					return ESP_FAIL;
				}

				/* copy file */
				ESP_LOGE(TAG, "open('%s', O_WRONLY|O_CREAT|O_TRUNC)", fname);
				int fd = open(fname, O_WRONLY|O_CREAT|O_TRUNC, 0644);
				if (fd < 0) {
					ESP_LOGE(TAG, "failed to open file '%s': %d", fname, errno);
					return ESP_FAIL;
				}

				size_t s = local_file_header.uncompr_size;
				while (s > 0) {
					uint8_t buf[128];
					size_t sz = s > sizeof(buf) ? sizeof(buf) : s;
					res = reader(reader_obj, buf, sz);
					if (res == -1 || res != sz)
					{
						ESP_LOGE(TAG, "(%d) failed to read from flash", __LINE__);
						return ESP_FAIL;
					}
					s -= sz;

					uint8_t *ptr = buf;
					while (sz > 0) {
						int err = write(fd, ptr, sz);
						if (err <= 0)
						{
							ESP_LOGE(TAG, "failed to write to fat");
							return ESP_FAIL;
						}
						ptr = &ptr[err];
						sz -= err;
					}
				}

				close(fd);

				if (local_file_header.compr_method == 8) { // deflated
					// check if we're at the end of the stream
					uint8_t read_end;
					ssize_t res = reader(reader_obj, &read_end, 1);
					if (res != 0) // should be 'end-of-stream'
					{
						ESP_LOGE(TAG, "(%d) failed to read from flash", __LINE__);
						return ESP_FAIL;
					}
				}
			}
		} else if (pk_sig == 0x02014b50 || pk_sig == 0x06054b50) { // directory object or end-of-directory object
			ESP_LOGE(TAG, "end of zip reached");
			break;
		} else if (first_chunk) {
			ESP_LOGE(TAG, "no preseed .zip found");
			return ESP_OK;
		} else {
			ESP_LOGE(TAG, "unknown zip object type 0x%08x", pk_sig);
			return ESP_FAIL;
		}
		first_chunk = false;
	}
	free(dr);

	// clear first page to avoid double unpacking
	int res = spi_flash_erase_sector((part_ota1->address + 4096) / SPI_FLASH_SEC_SIZE);
	if (res != ESP_OK) return res;
	
	printf("ZIP file extraction done!\n");
	return ESP_OK;
}
