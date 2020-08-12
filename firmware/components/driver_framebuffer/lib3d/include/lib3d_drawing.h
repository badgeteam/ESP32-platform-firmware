
#include "lib3d.h"

#ifdef __cplusplus
extern "C" {
#endif

// Renders a line in 3D space with depth buffer.
// Assumes pre-projected line.
void render3d_line(Window *window, vector_3d projected0, vector_3d projected1, uint32_t color);

// Renders a wireframe triangle in 3D space with depth buffer.
// Assumes pre-projected triangle.
L3D_INLINE void render3d_tri_wireframe(Window *window, triangle_3d triangle);

#ifdef __cplusplus
}
#endif
