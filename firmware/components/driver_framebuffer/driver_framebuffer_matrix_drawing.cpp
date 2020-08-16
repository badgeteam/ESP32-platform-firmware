
#include "include/driver_framebuffer_internal.h"

#include "sdkconfig.h"

#include <math.h>

#define TAG "fb-drawing"

#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE

#ifdef CONFIG_G_NEW_TRIANGLE
void driver_framebuffer_triangle(Window* window, float x0, float y0, float x1, float y1, float x2, float y2, uint32_t color)
{
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
	}
	if (y2 < y1) { //ensure y2 is under y1
		//swap points 2 and 1
		temp = y1;
		y1 = y2;
		y2 = temp;
		temp = x1;
		x1 = x2;
		x2 = temp;
	}
	if (y2 < y0) { //ensure y2 is under y0
		//swap points 2 and 0
		temp = y0;
		y0 = y2;
		y2 = temp;
		temp = x0;
		x0 = x2;
		x2 = temp;
	}
	if (y1 < y0) { //ensure y1 is under y0 once more
		//swap points 1 and 0
		temp = y0;
		y0 = y1;
		y1 = temp;
		temp = x0;
		x0 = x1;
		x1 = temp;
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
				driver_framebuffer_setPixel(window, x, y, color);
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
				driver_framebuffer_setPixel(window, x, y, color);
			}
		}
	}
}
#endif

#ifdef CONFIG_G_NEW_TEXT
void driver_framebuffer_triangle_textured(Window* window, float x0, float y0, float x1, float y1, float x2, float y2, triangle_uv uv, void *shaderArgs, shader_2d shader)
{
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
		//also uv
		temp = uv.u0;
		uv.u0 = uv.u1;
		uv.u1 = temp;
		temp = uv.v0;
		uv.v0 = uv.v1;
		uv.v1 = temp;
	}
	if (y2 < y1) { //ensure y2 is under y1
		//swap points 2 and 1
		temp = y1;
		y1 = y2;
		y2 = temp;
		temp = x1;
		x1 = x2;
		x2 = temp;
		//also uv
		temp = uv.u2;
		uv.u2 = uv.u1;
		uv.u1 = temp;
		temp = uv.v2;
		uv.v2 = uv.v1;
		uv.v1 = temp;
	}
	if (y2 < y0) { //ensure y2 is under y0
		//swap points 2 and 0
		temp = y0;
		y0 = y2;
		y2 = temp;
		temp = x0;
		x0 = x2;
		x2 = temp;
		//also uv
		temp = uv.u0;
		uv.u0 = uv.u2;
		uv.u2 = temp;
		temp = uv.v0;
		uv.v0 = uv.v2;
		uv.v2 = temp;
	}
	if (y1 < y0) { //ensure y1 is under y0 once more
		//swap points 1 and 0
		temp = y0;
		y0 = y1;
		y1 = temp;
		temp = x0;
		x0 = x1;
		x1 = temp;
		//also uv
		temp = uv.u0;
		uv.u0 = uv.u1;
		uv.u1 = temp;
		temp = uv.v0;
		uv.v0 = uv.v1;
		uv.v1 = temp;
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
				// Sample texture using barycentric interpolation
				float baryA = ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) / baryTemp;
				float baryB = ((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)) / baryTemp;
				float baryC = 1 - baryA - baryB;
				float u = baryA * uv.u0 + baryB * uv.u1 + baryC * uv.u2;
				float v = baryA * uv.v0 + baryB * uv.v1 + baryC * uv.v2;
				// u = fmod(u, 1);
				// v = fmod(v, 1);
				if (u < 0) u++;
				if (v < 0) v++;
				// Because of these shaders, it'll be relatively easy to shade things like this is exciting, weird ways
				// If alpha blending is added, this is not done by the shader
				uint32_t color = (*shader)(u, v, x, y, shaderArgs);
				// Plot color
				if ((color & 0xff000000) == 0xff000000) {
					driver_framebuffer_setPixel(window, x, y, color & 0xffffff);
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
				// Sample texture using barycentric interpolation
				float baryA = ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) / baryTemp;
				float baryB = ((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)) / baryTemp;
				float baryC = 1 - baryA - baryB;
				float u = baryA * uv.u0 + baryB * uv.u1 + baryC * uv.u2;
				float v = baryA * uv.v0 + baryB * uv.v1 + baryC * uv.v2;
				// u = fmod(u, 1);
				// v = fmod(v, 1);
				if (u < 0) u++;
				if (v < 0) v++;
				// Because of these shaders, it'll be relatively easy to shade things like this is exciting, weird ways
				// If alpha blending is added, this is not done by the shader
				uint32_t color = (*shader)(u, v, x, y, shaderArgs);
				// Plot color
				if ((color & 0xff000000) == 0xff000000) {
					driver_framebuffer_setPixel(window, x, y, color & 0xffffff);
				}
			}
		}
	}
}
#endif

#ifdef CONFIG_G_NEW_RECT
void driver_framebuffer_quad(Window* window, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, uint32_t color)
{
	// This is easier to do if represented as triangles.
	driver_framebuffer_triangle(window, x0, y0, x1, y1, x2, y2, color);
	driver_framebuffer_triangle(window, x0, y0, x2, y2, x3, y3, color);
}
#endif

#ifdef CONFIG_G_NEW_CIRCLE
// Not actually a unit test
float circle_test_radius(matrix_stack_2d* stack, float radius)
{
	matrix_2d current = stack->current;
	matrix_2d rotation = matrix_2d_rotate(M_PI * 0.25);
	float maxSqr = 0;
	float x = 0;
	float y = radius;
	matrix_2d_transform_point(current, &x, &y);
	float sqrDist = x * x + y * y;
	if (sqrDist > maxSqr) {
		maxSqr = sqrDist;
	}
	current = matrix_2d_multiply(current, rotation);
	maxSqr = 0;
	x = 0;
	y = radius;
	matrix_2d_transform_point(current, &x, &y);
	sqrDist = x * x + y * y;
	if (sqrDist > maxSqr) {
		maxSqr = sqrDist;
	}
	current = matrix_2d_multiply(current, rotation);
	maxSqr = 0;
	x = 0;
	y = radius;
	matrix_2d_transform_point(current, &x, &y);
	sqrDist = x * x + y * y;
	if (sqrDist > maxSqr) {
		maxSqr = sqrDist;
	}
	return sqrt(maxSqr);
}

void driver_framebuffer_circle_new(Window* window, matrix_stack_2d* stack, float x, float y, float radius, float startAngle, float endAngle, bool fill, uint32_t color)
{
	// Test the scale of the stack so as to have enough precision to fool the viewer
	float effectiveCircumfrence = circle_test_radius(stack, radius) * M_PI;
	int nSteps = effectiveCircumfrence < 80 ? (int) (effectiveCircumfrence / 1.7) : 60;
	float anglePerStep = (endAngle - startAngle) / (float) nSteps;
	// Make a copy of the matrix for later use
	matrix_2d current = stack->current;
	// Apply this multiple times instead of slow sin/cos
	matrix_2d rotationStep = matrix_2d_rotate(anglePerStep);
	current = matrix_2d_multiply(current, matrix_2d_translate(x, y));
	if (startAngle > 0.0000001) {
		// Rotate to the starting angle
		current = matrix_2d_multiply(current, matrix_2d_rotate(startAngle));
	}
	matrix_2d_transform_point(stack->current, &x, &y);
	// Start circling!
	if (fill) {
		float lastX = 0;
		float lastY = -radius;
		matrix_2d_transform_point(current, &lastX, &lastY);
		for (int i = 0; i < nSteps; i++) {
			float newX = 0;
			float newY = -radius;
			current = matrix_2d_multiply(current, rotationStep);
			matrix_2d_transform_point(current, &newX, &newY);
			driver_framebuffer_triangle(window, x, y, lastX, lastY, newX, newY, color);
			lastX = newX;
			lastY = newY;
		}
	}
	else
	{
		float lastX = 0;
		float lastY = -radius;
		matrix_2d_transform_point(current, &lastX, &lastY);
		for (int i = 0; i < nSteps; i++) {
			float newX = 0;
			float newY = -radius;
			current = matrix_2d_multiply(current, rotationStep);
			matrix_2d_transform_point(current, &newX, &newY);
			driver_framebuffer_line(window, (int) (lastX + 0.5), (int) (lastY + 0.5), (int) (newX + 0.5), (int) (newY + 0.5), color);
			lastX = newX;
			lastY = newY;
		}
	}
}
#endif

#ifdef CONFIG_G_NEW_TEXT
uint32_t driver_framebuffer_lerp_color(uint32_t _col0, uint32_t _col1, float part) {
	rgb_splitter_union col0 = {.together = _col0};
	rgb_splitter_union col1 = {.together = _col1};
	uint8_t red0 = col0.seperate.red;
	uint8_t red1 = col1.seperate.red;
	uint8_t redOut = red0 + (red1 - red0) * part;
	uint8_t green0 = col0.seperate.green;
	uint8_t green1 = col1.seperate.green;
	uint8_t greenOut = green0 + (green1 - green0) * part;
	uint8_t blue0 = col0.seperate.blue;
	uint8_t blue1 = col1.seperate.blue;
	uint8_t blueOut = blue0 + (blue1 - blue0) * part;
	uint8_t alpha0 = col0.seperate.alpha;
	uint8_t alpha1 = col1.seperate.alpha;
	uint8_t alphaOut = alpha0 + (alpha1 - alpha0) * part;
	// Note to self: only compiles if in the same order as declared
	rgb_splitter_union out = { .seperate = {
		.alpha = alphaOut,
		.red = redOut,
		.green = greenOut,
		.blue = blueOut
	}};
	return out.together;
}

// Looks nicer, is significantly slower than nolerp.
uint32_t shader_2d_lerp(float u, float v, int16_t x, int16_t y, void *args) {
	texture_2d *texture = (texture_2d *) args;
	u *= texture->width;
	v *= texture->width;
	int ui = (int) u;
	int vi = (int) v;
	int uip = (int) u + 1;
	int vip = (int) v + 1;
	int width = texture->width;
	int height = texture->height;
	ui %= width;
	vi %= height;
	uip %= width;
	vip %= height;
	float uf = u - ui;
	float vf = v - vi;
	uint32_t top = driver_framebuffer_lerp_color(texture->buffer[ui + vi * width], texture->buffer[uip + vi * width], uf);
	uint32_t bottom = driver_framebuffer_lerp_color(texture->buffer[ui + vip * width], texture->buffer[uip + vip * width], uf);
	return driver_framebuffer_lerp_color(top, bottom, vf);
}

// Will work for most things, is significantly faster than lerp.
uint32_t shader_2d_nolerp(float u, float v, int16_t x, int16_t y, void *args) {
	texture_2d *texture = (texture_2d *) args;
	int width = texture->width;
	int height = texture->height;
	int ui = (int) (u * (width + 0.00001) + 0.005);
	int vi = (int) (v * (height + 0.00001) + 0.005);
	ui %= width;
	vi %= height;
	return texture->buffer[ui + vi * width];
}
#endif

#endif
