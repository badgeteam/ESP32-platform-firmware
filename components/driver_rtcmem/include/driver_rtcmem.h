#ifndef DRIVER_RTCMEM_H
#define DRIVER_RTCMEM_H

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>

__BEGIN_DECLS

extern esp_err_t driver_rtcmem_int_write(int pos, int val);
extern esp_err_t driver_rtcmem_int_read(int pos, int* val);

extern esp_err_t driver_rtcmem_string_write(const char* str);
extern esp_err_t driver_rtcmem_string_read(const char** str);

extern esp_err_t driver_rtcmem_clear();

extern esp_err_t driver_rtcmem_init(void);

__END_DECLS

#endif // DRIVER_RTCMEM_H
