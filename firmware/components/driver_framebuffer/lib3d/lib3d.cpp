
#include "driver_framebuffer_internal.h"
#include "lib3d.h"

#ifdef CONFIG_LIB3D_ENABLE


/* PROJECTION */


// Projects a point in 3D space onto 2D space with perspecitve.
// DisplayPos X/Y is the position on screen to project around,
// DisplayPos Z is proportional to the field of view.
L3D_INLINE vector_2d project3d_perspec(matrix_stack_3d *stack, vector_3d projected, vector_3d displayPos) {
    float temp0 = displayPos.z / projected.z;
    return (vector_2d) {
        .x = temp0 * projected.x + displayPos.x,
        .y = temp0 * projected.y + displayPos.y
    };
}


#endif //CONFIG_LIB3D_ENABLE
