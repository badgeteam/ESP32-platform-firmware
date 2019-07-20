#include <sdkconfig.h>
#include <esp_err.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_heap_caps.h"
#include "include/val2pwm.h"
#include "include/i2s_parallel.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "include/driver_hub75.h"
#include "include/compositor.h"

#ifdef CONFIG_DRIVER_HUB75_ENABLE

//My display has each row swapped with its neighbour (so the rows are 2-1-4-3-6-5-8-7-...). If your display
//is more sane, uncomment this to get a good image.
#define DISPLAY_ROWS_SWAPPED 1

//This is the bit depth, per RGB subpixel, of the data that is sent to the display.
//The effective bit depth (in computer pixel terms) is less because of the PWM correction. With
//a bitplane count of 7, you should be able to reproduce an 16-bit image more or less faithfully, though.
#define BITPLANE_CNT 7

//64*32 RGB leds, 2 pixels per 16-bit value...
#define BITPLANE_SZ (8*32)

//Upper half RGB
#define BIT_R1 (1<<0)   //connected to GPIO2 here
#define BIT_G1 (1<<1)   //connected to GPIO15 here
#define BIT_B1 (1<<2)   //connected to GPIO4 here

#define BIT_A (1<<3)    //connected to GPIO5 here
#define BIT_B (1<<4)    //connected to GPIO18 here
#define BIT_C (1<<5)   //connected to GPIO19 here

#define BIT_LAT (1<<6) //connected to GPIO26 here
#define BIT_OE (1<<7)  //connected to GPIO25 here

int brightness=CONFIG_HUB75_DEFAULT_BRIGHTNESS;
int framerate=20;
Color *hub75_framebuffer = NULL;

bool driver_hub75_active;
i2s_parallel_buffer_desc_t bufdesc[2][1<<BITPLANE_CNT];
uint8_t *bitplane[2][BITPLANE_CNT];
int apos=0; //which frame in the animation we're on
int backbuf_id=0; //which buffer is the backbuffer, as in, which one is not active so we can write to it
//Get a pixel from the image at pix, assuming the image is a 64x32 8R8G8B image
//Returns it as an uint32 with the lower 24 bits containing the RGB values.
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#ifndef NULL
        #define NULL 0
#endif

void render16()
{
	//Fill bitplanes with the data for the current image
	for (int pl=0; pl<BITPLANE_CNT; pl++) {
		int mask=(1<<(8-BITPLANE_CNT+pl));         //bitmask for pixel data in input for this bitplane
		uint8_t *p=bitplane[backbuf_id][pl];         //bitplane location to write to
		for (unsigned int y=0; y<8; y++) {
			int lbits=0;         //Precalculate line bits of the *previous* line, which is the one we're displaying now
			if ((y-1)&1) lbits|=BIT_A;
			if ((y-1)&2) lbits|=BIT_B;
			if ((y-1)&4) lbits|=BIT_C;
			for (int fx=0; fx<32; fx++) {
				int x = fx^2;   //Apply correction. this fixes dma byte stream order
				int v = lbits;
				//Do not show image while the line bits are changing
				//Don't display for the first cycle to remove line bleed
				if (x<2 || x>=brightness) v|= BIT_OE;
				if (x==31) v|= BIT_LAT;         //latch on last bit...
				Color c1;
				int yreal = y;
				int xreal = 31-x;
				if (hub75_framebuffer) {
					c1 = hub75_framebuffer[yreal*CONFIG_HUB75_WIDTH+xreal];
				} else {
					c1.value = 0; //If no framebuffer available display is off.
				}
					
				if (valToPwm(c1.RGB[3]) & mask) v|= BIT_R1;
				if (valToPwm(c1.RGB[2]) & mask) v|= BIT_G1;
				if (valToPwm(c1.RGB[1]) & mask) v|= BIT_B1;                               
				
				//Save the calculated value to the bitplane memory
				*p++=v;
			}
		}
	}

	//Show our work!
	i2sparallel_flipBuffer(backbuf_id);
	backbuf_id^=1;
	//Bitplanes are updated, new image shows now.
}

void displayTask(void *pvParameter)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	while(driver_hub75_active) {
		vTaskDelayUntil( &xLastWakeTime, 100/framerate );
		if(compositor_status()) composite();
		render16();
	}
	vTaskDelete( NULL );
}

Color* getFrameBuffer()
{
        return hub75_framebuffer;
}

esp_err_t driver_hub75_init(void)
{
	//Change to set the global brightness of the display, range 1-63
	//Warning when set too high: Do not look into LEDs with remaining eye.
	#ifndef CONFIG_DRIVER_FRAMEBUFFER_ENABLE
	hub75_framebuffer = malloc(HUB75_BUFFER_SIZE);
	memset(hub75_framebuffer, 0, HUB75_BUFFER_SIZE);
	#endif
	
	for (int i=0; i<BITPLANE_CNT; i++) {
		for (int j=0; j<2; j++) {
			bitplane[j][i] = (uint8_t *) heap_caps_calloc(BITPLANE_SZ, sizeof(uint8_t), MALLOC_CAP_DMA);
			assert(bitplane[j][i] && "Can't allocate bitplane memory");
		}
	}

	//Do binary time division setup. Essentially, we need n of plane 0, 2n of plane 1, 4n of plane 2 etc, but that
	//needs to be divided evenly over time to stop flicker from happening. This little bit of code tries to do that
	//more-or-less elegantly.
	int times[BITPLANE_CNT]={0};
	for (int i=0; i<((1<<BITPLANE_CNT)-1); i++) {
		int ch=0;
		//Find plane that needs insertion the most
		for (int j=0; j<BITPLANE_CNT; j++) {
			if (times[j]<=times[ch]) ch=j;
		}
		//Insert the plane
		for (int j=0; j<2; j++) {
			bufdesc[j][i].memory=bitplane[j][ch];
			bufdesc[j][i].size=BITPLANE_SZ;
		}
		//Magic to make sure we choose this bitplane an appropriate time later next time
		times[ch]+=(1<<(BITPLANE_CNT-ch));
	}

	//End markers
	bufdesc[0][((1<<BITPLANE_CNT)-1)].memory=NULL;
	bufdesc[1][((1<<BITPLANE_CNT)-1)].memory=NULL;

	//Setup I2S
	i2sparallel_init(bufdesc[0], bufdesc[1]);

	compositor_init();
	#ifndef CONFIG_DRIVER_FRAMEBUFFER_ENABLE
	//compositor_setBuffer(getFrameBuffer());
	#endif

	driver_hub75_active = true;

	xTaskCreatePinnedToCore(
		&displayTask, /* Task function. */
		"display",    /* String with name of task. */
		1024,         /* Stack size in words. */
		NULL,         /* Parameter passed as input of the task */
		1,            /* Priority of the task. (Lower = more important) */
		NULL,         /* Task handle. */
		1             /* Core ID */
	);

	return ESP_OK;
}

void driver_hub75_set_brightness(int brightness_val)
{
	brightness = min(max(2, brightness_val), 30);
}

void driver_hub75_set_framerate(int framerate_val)
{
	framerate = min(max(1, framerate_val), 30);
}

void driver_hub75_switch_buffer(uint8_t* buffer)
{
	hub75_framebuffer = (Color*) buffer;
	compositor_setBuffer(hub75_framebuffer);
}

#else // DRIVER_HUB75_ENABLE
esp_err_t driver_hub75_init(void) { return ESP_OK; } // Dummy function, leave empty!
#endif
