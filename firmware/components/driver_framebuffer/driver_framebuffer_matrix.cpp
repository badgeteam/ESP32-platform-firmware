
#include "sdkconfig.h"
#include "include/driver_framebuffer_internal.h"
#include <math.h>

#ifdef CONFIG_DRIVER_FRAMEBUFFER_ENABLE

#ifdef CONFIG_G_MATRIX_ENABLE

/* HyperLazerDeathRays
     ~Me, probably. */

/* HELPER FUNCTIONS */

//creates a 2D matrix representing the given rotation in radians
matrix_2d matrix_2d_rotate(float angle) {
    float dSin = sin(angle);
    float dCos = cos(angle);
    return (matrix_2d){ .arr = {
            dCos, -dSin, 0,
            dSin, dCos, 0
    }};
}

//creates a 2D matrix representing the given translation
matrix_2d matrix_2d_translate(float x, float y) {
    return (matrix_2d){ .arr = {
            1, 0, x,
            0, 1, y
    }};
}

//creates a 2D matrix representing the given scaling
matrix_2d matrix_2d_scale(float x, float y) {
    return (matrix_2d){ .arr = {
            y, 0, 0,
            0, x, 0
    }};
}

// Potentially.
// matrix_3x2_2d matrix_2d_shear(float x, float y);

/* MATRIX OPERATIONS */

//checks whether or not the matrix is an identity matrix
//the identity matrix is a special transformation that represents no transformation being applied at all
bool matrix_2d_is_identity(matrix_2d matrix) {
    return matrix.var.a0 == 1 && matrix.var.a1 == 0 && matrix.var.a2 == 0 &&
           matrix.var.b0 == 0 && matrix.var.b1 == 1 && matrix.var.b2 == 0;
}

/*
 * 
 * Matrix multiplication:
 * 
 * [a  b  c] [p  q  r] [ap + bs + cv   aq + bt + cw   ar + bu + cx]
 * [d  e  f]*[s  t  u]=[dp + es + fv   dq + et + fw   dr + eu + fx]
 * [g  h  i] [v  w  x] [gp + hs + iv   gq + ht + iw   gr + hu + ix]
 * 
 * [a  b  c] [p]  [a]  [b]  [c] [ap + bs + cv]
 * [d  e  f]*[s]=p[d]+s[e]+v[f]=[dp + es + fv]
 * [g  h  i] [v]  [g]  [h]  [i] [gp + hs + iv]
 * 
 * [a  b  c] [q]  [a]  [b]  [c] [aq + bt + cw]
 * [d  e  f]*[t]=q[d]+t[e]+w[f]=[dq + et + fw]
 * [g  h  i] [w]  [g]  [h]  [i] [gq + ht + iw]
 * 
 * [a  b  c] [r]  [a]  [b]  [c] [ar + bu + cx]
 * [d  e  f]*[u]=r[d]+u[e]+x[f]=[dr + eu + fx]
 * [g  h  i] [x]  [g]  [h]  [i] [gr + hu + ix]
 * 
 * g=0, h=0, i=1
 * v=0, w=0, x=1
 * 
 * [a  b  c] [p  q  r] [ap + bs   aq + bt   ar + bu + c]
 * [d  e  f]*[s  t  u]=[dp + es   dq + et   dr + eu + f]
 * 
 */
//performs a matrix multiplication, internally factors in the bottom row which is omitted in storage
//this method is optimised for 2D
//TODO: potentially convert to assembly for even faster hyperspeeds
matrix_2d matrix_2d_multiply(matrix_2d left, matrix_2d right) {
	return (matrix_2d) { .arr = {
		left.var.a0*right.var.a0 + left.var.a1*right.var.b0, left.var.a0*right.var.a1 + left.var.a1*right.var.b1, left.var.a0*right.var.a2 + left.var.a1*right.var.b2 + left.var.a2, 
		left.var.b0*right.var.a0 + left.var.b1*right.var.b0, left.var.b0*right.var.a1 + left.var.b1*right.var.b1, left.var.b0*right.var.a2 + left.var.b1*right.var.b2 + left.var.b2
	}};
}

/*
 * 
 * Matrix-vector multiplication:
 * 
 * [a  b  c] [p]  [a]  [b]  [c] [ap + bq + cr]
 * [d  e  f]*[q]=p[d]+q[e]+r[f]=[dp + eq + fr]
 * [g  h  i] [r]  [g]  [h]  [i] [gp + hq + ir]
 * 
 * g=0, h=0, i=i
 * r=1
 * 
 * [a  b  c] [p]  [a]  [b] [c] [ap + bq + c]
 * [d  e  f]*[q]=p[d]+q[e]+[f]=[dp + eq + f]
 * 
 * [0  -1  20] [0]  [0]  [-1] [20] [0 + 0 + 20]
 * [1   0  30]*[0]=0[1]+0[0 ]+[30]=[0 + 0 + 30]
 * 
 */
//transforms the point according to the matrix
//this method is optimised for 2D
//TODO: potentially convert to assembly for even faster hyperspeeds
void matrix_2d_transform_point(matrix_2d matrix, float *x, float *y) {
    float xIn = *x;
    float yIn = *y;
    x[0] = matrix.var.a0*xIn + matrix.var.a1*yIn + matrix.var.a2;
    y[0] = matrix.var.b0*xIn + matrix.var.b1*yIn + matrix.var.b2;
}

//unlinks and deletes the entire stack
void matrix_stack_2d_unlink_all(matrix_2d_link *link) {
    matrix_2d_link *next;
    while (link != NULL) {
        next = link->next;
        delete link;
        link = next;
    }
}

/* STACK OPERATIONS */
//making the stack part of the matrix stack
//see a stack as a literal stack of paper in a box
//you can only access the top i.e. the one you just placed, and as such a stack is first-on-last-off storage
//push: take the current transformation and put it on the stack
//pop: remove the top from the stack and set the current transformation to it
//init: initialise the stack as empty and set the current matrix to identity
//clear: clear the entire matrix stack, including the current matrix, but assumes that the stack is already initialised

//initialises the given matrix stack so as to be ready for use
void matrix_stack_2d_init(matrix_stack_2d *stack) {
    stack->capacity = CONFIG_MATRIX_STACK_SIZE;
    stack->current = matrix_2d_identity();
    stack->matrices = NULL;
    stack->size = 0;
}

//clears the matrix stack, including the current matrix
//WARNING: This assumes the stack is already initialised!
void matrix_stack_2d_clear(matrix_stack_2d *stack) {
    //unlink all matrices
    matrix_stack_2d_unlink_all(stack->matrices);
    stack->matrices = NULL;

    stack->size = 0;
    stack->capacity = CONFIG_MATRIX_STACK_SIZE;
    stack->current = matrix_2d_identity();
}

//returns 1 if the stack would become too big
esp_err_t matrix_stack_2d_push(matrix_stack_2d *stack) {
    if (stack->size >= stack->capacity) {
        return 1;
    }
    //enlink current matrix
    matrix_2d_link *next = stack->matrices;
    stack->matrices = new matrix_2d_link();
    stack->matrices->matrix = stack->current;
    stack->matrices->next = next;

    stack->size ++;
    return ESP_OK;
}

//returns 1 if the stack is already empty
esp_err_t matrix_stack_2d_pop(matrix_stack_2d *stack) {
    if (stack->size <= 0 || stack->matrices == NULL) {
        stack->size = 0;
        return 1;
    }
    //unlink top matrix
    stack->current = stack->matrices->matrix;
    stack->matrices = stack->matrices->next;

    stack-> size --;
    return ESP_OK;
}

#endif //CONFIG_G_MATRIX_ENABLE

#endif //CONFIG_DRIVER_FRAMEBUFFER_ENABLE

