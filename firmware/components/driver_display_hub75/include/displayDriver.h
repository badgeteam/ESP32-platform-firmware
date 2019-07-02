#ifndef DISPLAYDRIVER_H
#define DISPLAYDRIVER_H

#include "compositor.h"
  
  
void displayDriver_init();
Color* getFrameBuffer();
void render16();
void setBrightness(int brightness_val);
void setFramerate(int framerate_val);
void displayTask(void *pvParameter);


#endif
