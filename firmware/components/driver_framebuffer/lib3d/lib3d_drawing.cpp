
#include "driver_framebuffer_internal.h"
#include "lib3d.h"

#ifdef CONFIG_LIB3D_ENABLE


// Renders a line in 3D space with depth buffer.
// Assumes pre-projected line.
void render3d_line(Window *window, vector_3d p0, vector_3d p1, uint32_t color) {
    //TODO: literally anything else
    driver_framebuffer_line(window, (int16_t) p0.x, (int16_t) p0.y, (int16_t) p1.x, (int16_t) p1.y, color);
}

// Renders a wireframe triangle in 3D space with depth buffer.
// Assumes pre-projected triangle.
L3D_INLINE void render3d_tri_wireframe(Window *window, triangle_3d triangle) {
    vector_3d p0 = triangle->p0;
    vector_3d p1 = triangle->p1;
    vector_3d p2 = triangle->p2;
    render3d_line(window, p0, p1, triangle.color);
    render3d_line(window, p1, p2, triangle.color);
    render3d_line(window, p2, p0, triangle.color);
}


#endif //CONFIG_LIB3D_ENABLE
