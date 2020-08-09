
#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union rgb_splitter_union_t {			// Used to seperate ARGB more efficiently due to the way CPUs work
	struct {
		uint8_t alpha;
		uint8_t red;
		uint8_t green;
		uint8_t blue;
	} seperate;
	uint32_t together;
} rgb_splitter_union;

typedef struct texture_2d_t {
	uint32_t *buffer;
	int16_t width;
	int16_t height;
    bool interpolation;
} texture_2d;

//shader_2d(float u, float v, int16_t x, int16_t y, void *args)
typedef uint32_t(*shader_2d)(float, float, int16_t, int16_t, void*);

typedef struct triangle_uv_t {
	float u0, v0;
	float u1, v1;
	float u2, v2;
} triangle_uv;

void driver_framebuffer_triangle(Window* window, float x0, float y0, float x1, float y1, float x2, float y2, uint32_t color);

void driver_framebuffer_triangle_textured(Window* window, float x0, float y0, float x1, float y1, float x2, float y2, triangle_uv uv, void *shaderArgs, shader_2d shader);
/* Draws a triangle with texture, texture format is up for debate. */

void driver_framebuffer_quad(Window* window, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, uint32_t color);
/* Draws a four-pointed polygon. */

void driver_framebuffer_circle_new(Window* window, matrix_stack_2d* stack, float x0, float y0, float r, float a0, float a1, bool fill, uint32_t color);
/* Draw a circle (filled or only the outline) at center point (x0, y0) with a radius r starting at angle a0 and ending at angle a1. */

uint32_t driver_framebuffer_lerp_color(uint32_t col0, uint32_t col1, float part);
/* Linear interpolation of two colors. */

uint32_t shader_2d_lerp(float u, float v, int16_t x, int16_t y, void *args);
/* Texture sampling shader with color interpolation. */

uint32_t shader_2d_nolerp(float u, float v, int16_t x, int16_t y, void *args);
/* Texture sampling shader without color interpolation. */

#ifdef __cplusplus
}
#endif

