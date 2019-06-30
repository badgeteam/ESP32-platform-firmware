#ifdef CONFIG_DRIVER_HUB75_ENABLE

#include "compositor.h"
#include "stdlib.h"
#include "string.h"

static uint8_t tinyFont[] = {
        0x00, 0x00, 0x00, 0x00, 0x00,// (space)
        0x00, 0x00, 0x5F, 0x00, 0x00,// !
        0x00, 0x07, 0x00, 0x07, 0x00,// "
        0x14, 0x7F, 0x14, 0x7F, 0x14,// #
        0x24, 0x2A, 0x7F, 0x2A, 0x12,// $
        0x23, 0x13, 0x08, 0x64, 0x62,// %
        0x36, 0x49, 0x55, 0x22, 0x50,// &
        0x00, 0x05, 0x03, 0x00, 0x00,// '
        0x00, 0x1C, 0x22, 0x41, 0x00,// (
        0x00, 0x41, 0x22, 0x1C, 0x00,// )
        0x08, 0x2A, 0x1C, 0x2A, 0x08,// *
        0x08, 0x08, 0x3E, 0x08, 0x08,// +
        0x00, 0x50, 0x30, 0x00, 0x00,// ,
        0x08, 0x08, 0x08, 0x08, 0x08,// -
        0x00, 0x60, 0x60, 0x00, 0x00,// .
        0x20, 0x10, 0x08, 0x04, 0x02,// /
        0x3E, 0x51, 0x49, 0x45, 0x3E,// 0
        0x00, 0x42, 0x7F, 0x40, 0x00,// 1
        0x42, 0x61, 0x51, 0x49, 0x46,// 2
        0x21, 0x41, 0x45, 0x4B, 0x31,// 3
        0x18, 0x14, 0x12, 0x7F, 0x10,// 4
        0x27, 0x45, 0x45, 0x45, 0x39,// 5
        0x3C, 0x4A, 0x49, 0x49, 0x30,// 6
        0x01, 0x71, 0x09, 0x05, 0x03,// 7
        0x36, 0x49, 0x49, 0x49, 0x36,// 8
        0x06, 0x49, 0x49, 0x29, 0x1E,// 9
        0x00, 0x36, 0x36, 0x36, 0x00,// :
        0x00, 0x56, 0x36, 0x00, 0x00,// ;
        0x00, 0x08, 0x14, 0x22, 0x41,// <
        0x14, 0x14, 0x14, 0x14, 0x14,// =
        0x41, 0x22, 0x14, 0x08, 0x00,// >
        0x02, 0x01, 0x51, 0x09, 0x06,// ?
        0x32, 0x49, 0x79, 0x41, 0x3E,// @
        0x7E, 0x11, 0x11, 0x11, 0x7E,// A
        0x7F, 0x49, 0x49, 0x49, 0x36,// B
        0x3E, 0x41, 0x41, 0x41, 0x22,// C
        0x7F, 0x41, 0x41, 0x22, 0x1C,// D
        0x7F, 0x49, 0x49, 0x49, 0x41,// E
        0x7F, 0x09, 0x09, 0x01, 0x01,// F
        0x3E, 0x41, 0x41, 0x51, 0x32,// G
        0x7F, 0x08, 0x08, 0x08, 0x7F,// H
        0x00, 0x41, 0x7F, 0x41, 0x00,// I
        0x20, 0x40, 0x41, 0x3F, 0x01,// J
        0x7F, 0x08, 0x14, 0x22, 0x41,// K
        0x7F, 0x40, 0x40, 0x40, 0x40,// L
        0x7F, 0x02, 0x04, 0x02, 0x7F,// M
        0x7F, 0x04, 0x08, 0x10, 0x7F,// N
        0x3E, 0x41, 0x41, 0x41, 0x3E,// O
        0x7F, 0x09, 0x09, 0x09, 0x06,// P
        0x3E, 0x41, 0x51, 0x21, 0x5E,// Q
        0x7F, 0x09, 0x19, 0x29, 0x46,// R
        0x46, 0x49, 0x49, 0x49, 0x31,// S
        0x01, 0x01, 0x7F, 0x01, 0x01,// T
        0x3F, 0x40, 0x40, 0x40, 0x3F,// U
        0x1F, 0x20, 0x40, 0x20, 0x1F,// V
        0x7F, 0x20, 0x18, 0x20, 0x7F,// W
        0x63, 0x14, 0x08, 0x14, 0x63,// X
        0x03, 0x04, 0x78, 0x04, 0x03,// Y
        0x61, 0x51, 0x49, 0x45, 0x43,// Z
        0x00, 0x00, 0x7F, 0x41, 0x41,// [
        0x02, 0x04, 0x08, 0x10, 0x20,// "\"
        0x41, 0x41, 0x7F, 0x00, 0x00,// ]
        0x04, 0x02, 0x01, 0x02, 0x04,// ^
        0x40, 0x40, 0x40, 0x40, 0x40,// _
        0x00, 0x01, 0x02, 0x04, 0x00,// `
        0x20, 0x54, 0x54, 0x54, 0x78,// a
        0x7F, 0x48, 0x44, 0x44, 0x38,// b
        0x38, 0x44, 0x44, 0x44, 0x20,// c
        0x38, 0x44, 0x44, 0x48, 0x7F,// d
        0x38, 0x54, 0x54, 0x54, 0x18,// e
        0x08, 0x7E, 0x09, 0x01, 0x02,// f
        0x08, 0x14, 0x54, 0x54, 0x3C,// g
        0x7F, 0x08, 0x04, 0x04, 0x78,// h
        0x00, 0x44, 0x7D, 0x40, 0x00,// i
        0x20, 0x40, 0x44, 0x3D, 0x00,// j
        0x00, 0x7F, 0x10, 0x28, 0x44,// k
        0x00, 0x41, 0x7F, 0x40, 0x00,// l
        0x7C, 0x04, 0x18, 0x04, 0x78,// m
        0x7C, 0x08, 0x04, 0x04, 0x78,// n
        0x38, 0x44, 0x44, 0x44, 0x38,// o
        0x7C, 0x14, 0x14, 0x14, 0x08,// p
        0x08, 0x14, 0x14, 0x18, 0x7C,// q
        0x7C, 0x08, 0x04, 0x04, 0x08,// r
        0x48, 0x54, 0x54, 0x54, 0x20,// s
        0x04, 0x3F, 0x44, 0x40, 0x20,// t
        0x3C, 0x40, 0x40, 0x20, 0x7C,// u
        0x1C, 0x20, 0x40, 0x20, 0x1C,// v
        0x3C, 0x40, 0x30, 0x40, 0x3C,// w
        0x44, 0x28, 0x10, 0x28, 0x44,// x
        0x0C, 0x50, 0x50, 0x50, 0x3C,// y
        0x44, 0x64, 0x54, 0x4C, 0x44,// z
        0x00, 0x08, 0x36, 0x41, 0x00,// {
        0x00, 0x00, 0x7F, 0x00, 0x00,// |
        0x00, 0x41, 0x36, 0x08, 0x00,// }
        0x08, 0x08, 0x2A, 0x1C, 0x08,// ->
        0x08, 0x1C, 0x2A, 0x08, 0x08 // <-
};

#define C_SM 0xFFFFFFFF

static uint32_t smiley[] = {
        0, 0, C_SM, C_SM, C_SM, C_SM, 0, 0,
        0, C_SM, 0, 0, 0, 0, C_SM, 0, 
        C_SM, 0, C_SM, 0, 0, C_SM, 0, C_SM,
        C_SM, 0, 0, 0, 0, 0, 0, C_SM,
        C_SM, 0, 0, C_SM, C_SM, 0, 0, C_SM,
        C_SM, 0, C_SM, 0, 0, C_SM, 0, C_SM,
        0, C_SM, 0, 0, 0, 0, C_SM, 0,
        0, 0, C_SM, C_SM, C_SM, C_SM, 0, 0
};

bool enabled = true;

Color background;
Color *buffer;
renderTask_t *head = NULL;

void addTask(renderTask_t *node);
void addColor(Color *target, Color *color);
void renderImage(uint8_t *image, int x, int y, int sizeX, int sizeY);
void renderCharCol(uint8_t ch, Color color, int x, int y);
void renderText(char *text, Color color, int x, int y, int sizeX, int skip);


Color genColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        Color color;
        color.RGB[0] = r;
        color.RGB[1] = g;
        color.RGB[2] = b;
        color.RGB[3] = a;
        return color;
}

void compositor_init() {
        background.value = 0;
}

/*
Clears the render list. Keeps the background
 */
void compositor_clear() {
        renderTask_t *node = head;
        renderTask_t *next;
        while(node != NULL) {
                next = node->next;
                if(node->id == 1) {
                        free(node->payload);
                }
                if(node->id == 3) {
                        animation_t *gif = (animation_t *) node->payload;
                        free(gif->gif);
                }
                if(node->id == 2 || node->id == 3) {
                        free(node->payload);
                }
                free(node);
                node = next;
        }
        head = NULL;
}

/*
* Sets the background color of the display.
*/
void compositor_setBackground(Color color) {
        background = color;
}

void addTask(renderTask_t *node) {
        if(head == NULL) {
                head = node;
        } else {
                renderTask_t *pos = head;
                while(pos->next != NULL) pos = pos->next;
                pos->next = node;
        }
}

void compositor_addText(char *text, Color color, int x, int y) {
        renderTask_t *node = (renderTask_t *) malloc(sizeof(renderTask_t));
        node->payload = text;
        node->color = color;
        node->x = x;
        node->y = y;
        node->next = NULL;
        node->id = 0;
        addTask(node);
}

/*
* Add text to be rendered but also scroll it from right to left.
*
* text is the text to be rendered
* color is the color to be rendered
* x, y is the coordinate of the top left corner of the text block. each character is 8 pixels high and 5 pixels wide
* sizeX is the length over which text should be drawn
*/
void compositor_addScrollText(char *text, Color color, int x, int y, int sizeX) {
        scrollText_t *scroll = (scrollText_t *) malloc(sizeof(scrollText_t));
        scroll->text = text;
        scroll->speed = 1;
        scroll->skip = 0;
        renderTask_t *node = (renderTask_t *) malloc(sizeof(renderTask_t));
        node->payload = scroll;
        node->id = 2;
        node->x = x;
        node->y = y;
        node->sizeX = sizeX;
        node->color = color;
        node->next = NULL;
        addTask(node);
}

/*
* Renders an image
*
* image is pointer to your image
* x,y is the coordinate for the top left corner
* width, length is width and length of the image
*/
void compositor_addImage(uint8_t *image, int x, int y, int width, int length) {
        renderTask_t *node = (renderTask_t *) malloc(sizeof(renderTask_t));
        node->payload = image;
        node->x = x;
        node->y = y;
        node->sizeX = width;
        node->sizeY = length;
        node->next = NULL;
        node->id = 1;
        addTask(node);
}

/*
* Renders an animation
*
* image is pointer to your image
* x,y is the coordinate for the top left corner
* width, length is width and length of the image
* numframes is the number of frames in the animation
*/
void compositor_addAnimation(uint8_t *image, int x, int y, int width, int length, int numFrames) {
        animation_t *gif = (animation_t *) malloc(sizeof(animation_t));
        gif->gif = image;
        gif->showFrame = 0;
        gif->numberFrames = numFrames;
        renderTask_t *node = (renderTask_t *) malloc(sizeof(renderTask_t));
        node->payload = gif;
        node->x = x;
        node->y = y;
        node->sizeX = width;
        node->sizeY = length;
        node->next = NULL;
        node->id = 3;
        addTask(node);
}

void addColor(Color *target, Color *color) {
        target->RGB[0] = color->RGB[0] + (255-color->RGB[3])*target->RGB[0]/255;
        target->RGB[1] = color->RGB[1] + (255-color->RGB[3])*target->RGB[1]/255;
        target->RGB[2] = color->RGB[2] + (255-color->RGB[3])*target->RGB[2]/255;
}

void renderImage(uint8_t *image, int x, int y, int sizeX, int sizeY) {
        int xreal, yreal;
        for(int py=0; py<sizeY; py++) {
                yreal = y + py;
                for(int px=0; px<sizeX; px++) {
                        xreal = x + px;
                        if(yreal >= 0 && yreal < HEIGHT && xreal >= 0 && xreal < WIDTH) {
                                addColor(&buffer[yreal*WIDTH+xreal], (Color *)&image[(py*sizeX+px)*4]);
                        }
                }
        }
}

void renderCharCol(uint8_t ch, Color color, int x, int y) {
        for(int py = y; py<y+7; py++) {
                if(py >= 0 && py < HEIGHT && x >= 0 && x < WIDTH) {
                        if((ch & (1<<(py-y))) != 0) addColor(&buffer[py*WIDTH+x], &color);
                }
        }
}

void renderText(char *text, Color color, int x, int y, int sizeX, int skip) {
        int endX = x+sizeX;
        while(skip < 0) {
                x++;
                skip++;
        }
        for(int i = 0; i<strlen(text); i++) {
                uint8_t charId = (uint8_t)text[i] - 32;
                for(int i = 0; i<5; i++) {
                        if(skip == 0) {
                                uint8_t cs = tinyFont[charId*5+i];
                                renderCharCol(cs, color, x, y);
                                x++;
                                if(sizeX > 0 && x >= endX) return;
                        } else {
                                skip--;
                        }
                }
                if(skip == 0) x++; //If started printing insert blank line
                else skip--; //If not decrease the number to skip by one to make it fluid
        }
}

void display_crash() {
        enabled = false;
        Color blue;
        blue.value = 0x00AA7010;
        Color white;
        white.value = 0xFFFFFFFF;
        for(int x=0; x<WIDTH; x++) {
                for(int y=0; y<HEIGHT; y++) {
                        buffer[y*WIDTH+x] = blue;
                }
        }
        renderImage((uint8_t *) smiley, 24, 0, 8, 8);   
        renderText("FML", white, 0, 0, -1, 0);     
}

void composite() {
        //Setting the background color
        for(int x=0; x<WIDTH; x++) {
                for(int y=0; y<HEIGHT; y++) {
                        buffer[y*WIDTH+x] = background;
                }
        }
        renderTask_t *node = head;
        while(node != NULL) {
                if(node->id == 0) { //Render text
                        renderText((char *)node->payload, node->color, node->x, node->y, -1, 0);
                } else if(node->id == 1) {  //Render image
                        renderImage((uint8_t *)node->payload, node->x, node->y, node->sizeX, node->sizeY);
                } else if(node->id == 2) {  //Render scrolling text
                        scrollText_t *scroll = (scrollText_t *) node->payload;
                        renderText(scroll->text, node->color, node->x, node->y, node->sizeX, scroll->skip);
                        scroll->skip++;
                        if(scroll->skip == strlen(scroll->text)*6+6) scroll->skip = -node->sizeX;
                } else if(node->id == 3) {//Render animation
                        animation_t *gif = (animation_t *) node->payload;
                        int index = node->sizeX*node->sizeY*4*gif->showFrame;
                        renderImage(&(gif->gif[index]), node->x, node->y, node->sizeX, node->sizeY);
                        gif->showFrame++;
                        if(gif->showFrame == gif->numberFrames) gif->showFrame = 0;
                }
                node = node->next;
        }

}

void compositor_setBuffer(Color* framebuffer) {
        buffer = framebuffer;
}



void compositor_enable() {
        enabled = true;
}

void compositor_disable() {
        enabled = false;
}

bool compositor_status() {
        return enabled;
}


#endif
