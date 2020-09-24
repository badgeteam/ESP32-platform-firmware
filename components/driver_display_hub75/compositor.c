#include <sdkconfig.h>

#ifdef CONFIG_DRIVER_HUB75_ENABLE
#include "stdlib.h"
#include "string.h"
#include "include/font_7x5.h"
#include "include/font_6x3.h"
#include "include/compositor.h"
#include "esp_log.h"


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

#define N_FONTS 2
int font_index = 0;
void (*font_render_char[])(uint8_t charId, Color color, int *x, int y, int endX, int *skip) = {&renderChar_7x5, &renderChar_6x3};
int (*font_char_width[])(uint8_t charId) = {&getCharWidth_7x5, &getCharWidth_6x3};

void addTask(renderTask_t *node);
void renderImage(uint8_t *image, int x, int y, int sizeX, int sizeY);
void renderCharCol(uint8_t ch, Color color, int x, int y);
void renderText(char *text, Color color, int x, int y, int sizeX, int skip, bool firstshow);


Color genColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        Color color;
        color.RGB[0] = a;
        color.RGB[1] = b;
        color.RGB[2] = g;
        color.RGB[3] = r;
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
                if(node->id == 0) {
                        free(node->payload);
                } else if(node->id == 1) {
                        free(node->payload);
                } else if(node->id == 2) {
                        scrollText_t *t = (scrollText_t *) node->payload;
                        free(t->text);
                        free(node->payload);
                } else if(node->id == 3) {
                        animation_t *gif = (animation_t *) node->payload;
                        free(gif->gif);
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
        char *text_store = malloc(strlen(text)+1);
        strcpy(text_store, text);
        renderTask_t *node = (renderTask_t *) malloc(sizeof(renderTask_t));
        node->payload = text_store;
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
	char *text_store = malloc(strlen(text)+1);
	strcpy(text_store, text);
	scrollText_t *scroll = (scrollText_t *) malloc(sizeof(scrollText_t));
	scroll->text = text_store;
	scroll->speed = 1;
	scroll->skip = -10;
	scroll->firstshow = true;
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

void compositor_setPixel(int x, int y, Color color) {
	if (!buffer) return;
	Color *target = &buffer[y*CONFIG_HUB75_WIDTH+x];
	target->RGB[1] = color.RGB[1] + (255-color.RGB[0])*target->RGB[1]/255;
	target->RGB[2] = color.RGB[2] + (255-color.RGB[0])*target->RGB[2]/255;
	target->RGB[3] = color.RGB[3] + (255-color.RGB[0])*target->RGB[3]/255;
}

void renderImage(uint8_t *image, int x, int y, int sizeX, int sizeY) {
	int xreal, yreal;
	for(int py=0; py<sizeY; py++) {
		yreal = y + py;
		for(int px=0; px<sizeX; px++) {
			xreal = x + px;
			if(yreal >= 0 && yreal < CONFIG_HUB75_HEIGHT && xreal >= 0 && xreal < CONFIG_HUB75_WIDTH) {
				compositor_setPixel(xreal, yreal, *((Color *)&image[(py*sizeX+px)*4]));
			}
		}
	}
}

void renderText(char *text, Color color, int x, int y, int sizeX, int skip, bool firstshow) {
	int endX = sizeX > 0 ? x+sizeX : CONFIG_HUB75_WIDTH - 1;
	if(skip < 0) {
		if (!firstshow)
		{
			x += -skip;
		}
		else
		{
		    x += 1;
		}
		skip = 0;
	}
	for(int i = 0; i<strlen(text); i++) {
		uint8_t charId = (uint8_t)text[i] - 32;
		(*font_render_char[font_index])(charId, color, &x, y, endX, &skip);
		if(skip == 0) x++; //If started printing insert blank line
		else skip--; //If not decrease the number to skip by one to make it fluid
	}
}

unsigned int compositor_getTextWidth(char *text) {
	int width = 0;

	for(int i = 0; i<strlen(text); i++) {
		uint8_t charId = (uint8_t)text[i] - 32;
		width += (*font_char_width[font_index])(charId);
	}
	if (strlen(text) > 1)
		width += strlen(text);
	return width;
}

void compositor_setFont(int index) {
	if(index < 0 || index >= N_FONTS) return;
	font_index = index;
}


void display_crash() {
	enabled = false;
	if (!buffer) return;
	Color blue;
	blue.value = 0x1070AA00;
	Color white;
	white.value = 0xFFFFFFFF;
	for(int x=0; x<CONFIG_HUB75_WIDTH; x++) {
		for(int y=0; y<CONFIG_HUB75_HEIGHT; y++) {
			buffer[y*CONFIG_HUB75_WIDTH+x] = blue;
		}
	}
	renderImage((uint8_t *) smiley, 24, 0, 8, 8);   
	renderText("FML", white, 0, 0, -1, 0, false);     
}

void composite() {
	if (!buffer) return;
	//Setting the background color
	for(int x=0; x<CONFIG_HUB75_WIDTH; x++) {
			for(int y=0; y<CONFIG_HUB75_HEIGHT; y++) {
					buffer[y*CONFIG_HUB75_WIDTH+x] = background;
			}
	}
	renderTask_t *node = head;
	while(node != NULL) {
		if(node->id == 0) { //Render text
			renderText((char *)node->payload, node->color, node->x, node->y, -1, 0, false);
		} else if(node->id == 1) {  //Render image
			renderImage((uint8_t *)node->payload, node->x, node->y, node->sizeX, node->sizeY);
		} else if(node->id == 2) {  //Render scrolling text
			scrollText_t *scroll = (scrollText_t *) node->payload;
			renderText(scroll->text, node->color, node->x, node->y, node->sizeX, scroll->skip, scroll->firstshow);
			scroll->skip++;
			if(scroll->skip == strlen(scroll->text)*6+6) 
			{
				scroll->skip = -node->sizeX;
				scroll->firstshow = false;
			}
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
