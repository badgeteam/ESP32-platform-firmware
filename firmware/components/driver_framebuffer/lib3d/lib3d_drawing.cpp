
#include "driver_framebuffer_internal.h"
#include "lib3d.h"

#ifdef CONFIG_LIB3D_ENABLE


// Renders a line in 3D space with depth buffer.
// Assumes pre-projected line.
void render3d_line(Window *window, vector_3d p0, vector_3d p1, uint32_t color) {
	bool steep = abs(p1.y - p0.y) > abs(p1.x - p0.x);
    if (steep) {
        float xStep = (p1.x - p0.x) / (p1.y - p0.y);
        float yStep = p1.y > p0.y ? 1 : -1;
        int nSteps = (int) abs(p1.y - p0.y);
        float x = p0.x;
        float y = p0.y;
        for (int i = 0; i < nSteps; i++) {
            driver_framebuffer_setPixel(window, (int) (x + 0.5), (int) (y + 0.5), color);
            x += xStep;
            y += yStep;
        }
    }
    else
    {
        float yStep = (p1.y - p0.y) / (p1.x - p0.x);
        float xStep = p1.x > p0.x ? 1 : -1;
        int nSteps = (int) abs(p1.x - p0.x);
        float x = p0.x;
        float y = p0.y;
        for (int i = 0; i < nSteps; i++) {
            driver_framebuffer_setPixel(window, (int) (x + 0.5), (int) (y + 0.5), color);
            x += xStep;
            y += yStep;
        }
    }
}

// Renders a wireframe triangle in 3D space with depth buffer.
// Assumes pre-projected triangle.
L3D_INLINE void render3d_tri_wireframe(Window *window, triangle_3d triangle) {
    vector_3d p0 = triangle->a0;
    vector_3d p1 = triangle->a1;
    vector_3d p2 = triangle->a2;
    render3d_line(window, p0, p1, triangle.color);
    render3d_line(window, p1, p2, triangle.color);
    render3d_line(window, p2, p0, triangle.color);
}

void render3d_tri(Window *window, triangle_3d triangle) {
    depth_buffer_3d_t *depthBuffer = window->depth_buffer;
    uint32_t color = triangle.color;
    float x0 = triangle->a0.x;
    float y0 = triangle->a0.y;
    float z0 = triangle->a0.z;
    float x1 = triangle->a1.x;
    float y1 = triangle->a1.y;
    float z1 = triangle->a1.z;
    float x2 = triangle->a2.x;
    float y2 = triangle->a2.y;
    float z2 = triangle->a2.z;

	//sort the points such that point 0 is the top and point 2 is the bottom
	//lower number is higher on screen
	float temp;
	if (y1 < y0) { //ensure y1 is under y0
		//swap points 1 and 0
		temp = y0;
		y0 = y1;
		y1 = temp;
		temp = x0;
		x0 = x1;
		x1 = temp;
        temp = z0;
        z0 = z1;
        z1 = temp;
	}
	if (y2 < y1) { //ensure y2 is under y1
		//swap points 2 and 1
		temp = y1;
		y1 = y2;
		y2 = temp;
		temp = x1;
		x1 = x2;
		x2 = temp;
        temp = z1;
        z1 = z2;
        z2 = temp;
	}
	if (y2 < y0) { //ensure y2 is under y0
		//swap points 2 and 0
		temp = y0;
		y0 = y2;
		y2 = temp;
		temp = x0;
		x0 = x2;
		x2 = temp;
        temp = z0;
        z0 = z2;
        z2 = temp;
	}
	if (y1 < y0) { //ensure y1 is under y0 once more
		//swap points 1 and 0
		temp = y0;
		y0 = y1;
		y1 = temp;
		temp = x0;
		x0 = x1;
		x1 = temp;
        temp = z0;
        z0 = z1;
        z1 = temp;
	}
	
	// From point 0 to point 1
	float inc01 = (x1 - x0) / (y1 - y0);
	float add01 = (x0 / inc01 - y0) * inc01;
	if (inc01 == 0) add01 = x0;
	// From point 0 to point 2
	float inc02 = (x2 - x0) / (y2 - y0);
	float add02 = (x0 / inc02 - y0) * inc02;
	if (inc02 == 0) add02 = x0;
	// From point 1 to point 2
	float inc12 = (x2 - x1) / (y2 - y1);
	float add12 = (x1 / inc12 - y1) * inc12;
	if (inc12 == 0) add12 = x1;

	// Does not change per pixel in barycentric interpolation
	float baryTemp = (y1 - y2) * (x0 - x2) + (x2 - x1) * (y0 - y2);

	// Check whether we need to draw the top part
	int yCheck = (int) (y0 + 0.5);
	if ((float) yCheck + 0.5 <= y1) {
		// Draw top part
		int startY = (int) (y0 + 0.5);
		int endY = (int) (y1 + 0.5);
		for (int y = startY; y < endY; y++) {
			int startX = ((float) y + 0.5) * inc01 + add01 + 0.5;
			int endX = ((float) y + 0.5) * inc02 + add02 + 0.5;
			if (startX > endX) {
				int tmp = startX;
				startX = endX;
				endX = tmp;
			}
			for (int x = startX; x < endX; x++) {
				// Get Z using barycentric interpolation
				float baryA = ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) / baryTemp;
				float baryB = ((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)) / baryTemp;
				float baryC = 1 - baryA - baryB;
                float z = baryA * z0 + baryB * z1 + baryC * z2;
                float oldZ = depth3d_get(depthBuffer, x, y);
                // TODO: proper opacity handling
                if (z > oldZ) {
                    depth3d_set(depthBuffer, x, y, z);
				    driver_framebuffer_setPixel(window, x, y, color);
                }
			}
		}
	}
	// Check whether we need to draw the bottom part
	yCheck = (int) (y1 + 0.5);
	if ((float) yCheck + 0.5 <= y2) {
		// Draw bottom part
		int startY = (int) (y1 + 0.5);
		int endY = (int) (y2 + 0.5);
		for (int y = startY; y < endY; y++) {
			int startX = ((float) y + 0.5) * inc12 + add12 + 0.5;
			int endX = ((float) y + 0.5) * inc02 + add02 + 0.5;
			if (startX > endX) {
				int tmp = startX;
				startX = endX;
				endX = tmp;
			}
			for (int x = startX; x < endX; x++) {
				// Get Z using barycentric interpolation
				float baryA = ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) / baryTemp;
				float baryB = ((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)) / baryTemp;
				float baryC = 1 - baryA - baryB;
                float z = baryA * z0 + baryB * z1 + baryC * z2;
                float oldZ = depth3d_get(depthBuffer, x, y);
                // TODO: proper opacity handling
                if (z > oldZ) {
                    depth3d_set(depthBuffer, x, y, z);
				    driver_framebuffer_setPixel(window, x, y, color);
                }
			}
		}
	}
}


#endif //CONFIG_LIB3D_ENABLE
