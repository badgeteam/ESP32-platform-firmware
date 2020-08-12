
#include "driver_framebuffer_internal.h"
#include "lib3d_buffer.h"
#include "lib3d_drawing.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_LIB3D_INLINING
#define L3D_INLINE static inline
#else
#define L3D_INLINE
#endif


/* DATA TYPES */


typedef struct vector_3d_t {
    float x, y, z;
} vector_3d;

typedef struct vector_2d_t {
    float x, y;
} vector_2d;

typedef struct triangle_3d_t {
    // Untransformed, unprojected vertices.
    vector_3d_t *p0;
    vector_3d_t *p1
    vector_3d_t *p2;
    // First transformed, then projected vertices.
    vector_3d_t *a0;
    vector_3d_t *a1
    vector_3d_t *a2;
    //TODO: Something more to describe anything but wireframe rendering.
    uint32_t color;
} triangle_3d;

typedef struct model_3d_t {
    vector_3d_t *vertices;
    uint16_t num_vertices;
    triangle_3d_t *triangles;
    uint16_t num_triangles;
} model_3d;


/* PROJECTION */


// Projects a point in 3D space onto 2D space with perspecitve.
// Camera angle in trait-bryan format.
L3D_INLINE vector_2d project3d_perspec(matrix_stack_3d *stack, vector_3d projected, vector_3d camPos, vector_3d camAngle, vector_3d displayPos);


#ifdef __cplusplus
}
#endif
