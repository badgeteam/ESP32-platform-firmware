#ifndef DRIVER_FRI3D_H
#define DRIVER_FRI3D_H

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>

#define FRI3D_WIDTH  14
#define FRI3D_HEIGHT 5
#define FRI3D_BUFFER_SIZE FRI3D_WIDTH*FRI3D_HEIGHT

__BEGIN_DECLS

extern esp_err_t driver_fri3d_init(void);
extern esp_err_t driver_fri3d_write(const uint8_t *data);

__END_DECLS

#endif // DRIVER_FRI3D_H
