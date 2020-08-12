#include "lib3d.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MAGIC SAUCE */


#ifdef CONFIG_LIB3D_SMALLBUF
    // Smaller depth buffer: short (2 bytes per pixel)
    //   This way, the lowest 7 bits are the fraction.
    //   This increases the chance of Z-fighting; when two surfaces are very close or in the same exact spot, they fight over which is on top.

    L3D_INLINE int16_t _l3d_float_to_depth(float in);
    L3D_INLINE float _l3d_depth_to_float(int16_t in);
    #define L3D_FLOAT_TO_DEPTH(x) _l3d_float_to_depth(x)
    #define L3D_DEPTH_TO_FLOAT(x) _l3d_depth_to_float(x)
    typedef int16_t depth_buffer_type_t;
#else
    // Default depth buffer: float (4 bytes per pixel)
    //   This way, the precision is perfectly kept.
    //   This decreases the chance of Z-fighting; when two surfaces are very close or in the same exact spot, they fight over which is on top.

    #define L3D_FLOAT_TO_DEPTH(x) (x)
    #define L3D_DEPTH_TO_FLOAT(x) (x)
    typedef float depth_buffer_type_t;
#endif


/* DATA TYPES */


typedef struct depth_buffer_3d_t {
    depth_buffer_type_t *buffer;
    uint16_t width;
    uint16_t height;
    bool is_clear;
} depth_buffer_3d;


/* FUNCTIONS */


// Slower, safe getting; with bounds check.
L3D_INLINE float depth3d_get(depth_buffer_3d_t *buf, int x, int y);
// Faster, but unsafe getting; without bounds check.
L3D_INLINE float depth3d_get_unsafe(depth_buffer_3d_t *buf, int x, int y);

// Slower, safe setting; with bounds check.
L3D_INLINE float depth3d_set(depth_buffer_3d_t *buf, int x, int y, float value);
// Faster, but unsafe setting; without bounds check.
L3D_INLINE float depth3d_set_unsafe(depth_buffer_3d_t *buf, int x, int y, float value);


#ifdef __cplusplus
}
#endif
