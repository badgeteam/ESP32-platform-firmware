#ifndef DRIVER_SDCARD_H
#define DRIVER_SDCARD_H

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>

__BEGIN_DECLS

extern bool driver_sdcard_is_mounted();
extern esp_err_t driver_sdcard_unmount();
extern esp_err_t driver_sdcard_mount(const char* mount_point, bool format_if_mount_failed);
extern esp_err_t driver_sdcard_init(void);

__END_DECLS

#endif // DRIVER_SDCARD_H
