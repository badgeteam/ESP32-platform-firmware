
#include "driver_framebuffer_internal.h"
#include "lib3d.h"

#ifdef CONFIG_LIB3D_ENABLE


/* MAGIC SAUCE */

#ifdef CONFIG_LIB3D_SMALLBUF
L3D_INLINE int16_t _l3d_float_to_depth(float in) {
    int16_t whole = (int) in;
    int16_t fraction = (in - whole) * 0x7F;
    return (whole << 7) | fraction;
}

L3D_INLINE float _l3d_depth_to_float(int16_t in) {
    float whole = in >> 7;
    float fraction = (in & 0x7F) / (float) 0x7F;
    return whole + fraction;
}
#endif


/* FUNCTIONS */


// Slower, safe getting; with bounds check.
L3D_INLINE float depth3d_get(depth_buffer_3d_t *buf, int x, int y) {
    if (x < 0 || y < 0 || x >= buf->width || y >= buf->height) {
        return 0;
    }
    else
    {
        return L3D_DEPTH_TO_FLOAT(buf[x + y * buf->width]);
    }
}

// Faster, but unsafe getting; without bounds check.
L3D_INLINE float depth3d_get_unsafe(depth_buffer_3d_t *buf, int x, int y) {
    return L3D_DEPTH_TO_FLOAT(buf[x + y * buf->width]);
}

// Slower, safe setting; with bounds check.
L3D_INLINE float depth3d_set(depth_buffer_3d_t *buf, int x, int y, float value) {
    if (x >= 0 && y >= 0 && x < buf->width && y < buf->height) {{
        buf[x + y * buf->width] = L3D_FLOAT_TO_DEPTH(value);
    }
}

// Faster, but unsafe setting; without bounds check.
L3D_INLINE float depth3d_set_unsafe(depth_buffer_3d_t *buf, int x, int y, float value) {
    buf[x + y * buf->width] = L3D_FLOAT_TO_DEPTH(value);
}


#endif //CONFIG_LIB3D_ENABLE
