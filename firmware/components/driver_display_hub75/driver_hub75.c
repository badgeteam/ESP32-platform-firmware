#include <sdkconfig.h>
#include <esp_err.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_heap_caps.h"
#include "include/val2pwm.h"
#include "driver/ledc.h"
#include "driver/adc.h"

#include "include/i2s_parallel.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "freertos/queue.h"
#include "include/driver_hub75.h"

#include "include/compositor.h"
#include "esp_log.h"

#ifdef CONFIG_DRIVER_HUB75_ENABLE

#define TAG "hub75"

//This is the bit depth, per RGB subpixel, of the data that is sent to the display.
//The effective bit depth (in computer pixel terms) is less because of the PWM correction. With
//a bitplane count of 7, you should be able to reproduce an 16-bit image more or less faithfully, though.
//Currently set to 5 to improve scrolling text.
#define BITPLANE_CNT 6

// This adds empty frames after each row. The last byte of the last frame will latch, blanking the
// row. This allows the led driver ICs to drain the charge of the led columns before moving onto
// a new row, drasticall reducing line bleeding.
#define NULLFRAMES 3

// Configuration for LED power rail regulation
#define UPPER   600
#define MID     350
#define LOWER   40
#define PCT_MID 200  // 78% of 255
#define PCT_MIN 25  // 10% of 255

//8*32 RGB leds, with a blank frame after it to suppress line bleeding
#define BITPLANE_SZ (8*32*(NULLFRAMES+1))

//Upper half RGB
#define BIT_R1 (1<<0)   //connected to GPIO2 here
#define BIT_G1 (1<<1)   //connected to GPIO15 here
#define BIT_B1 (1<<2)   //connected to GPIO4 here

#define BIT_A (1<<3)    //connected to GPIO5 here
#define BIT_B (1<<4)    //connected to GPIO18 here
#define BIT_C (1<<5)   //connected to GPIO19 here

#define BIT_LAT (1<<6) //connected to GPIO26 here
#define BIT_OE (1<<7)  //connected to GPIO25 here

static uint32_t total_intensity = 0;
int brightness=CONFIG_HUB75_DEFAULT_BRIGHTNESS;
int framerate=20;
Color *hub75_framebuffer = NULL;

bool driver_hub75_active; // Stops all compositing + DMA buffer updating
i2s_parallel_buffer_desc_t bufdesc[2][(1<<BITPLANE_CNT)];
uint8_t *bitplane[2][BITPLANE_CNT];
int backbuf_id=0; //which buffer is the backbuffer, as in, which one is not active so we can write to it
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#ifndef NULL
        #define NULL 0
#endif

void render16()
{
        total_intensity = 0;
        //Fill bitplanes with the data for the current image
        for (int plane=0; plane<BITPLANE_CNT; plane++) {
            for (unsigned int y=0; y<8; y++) {
                int lbits=0;         //Precalculate line bits of the *previous* line, which is the one we're displaying now
                if ((y-1)&1) lbits|=BIT_A;
                if ((y-1)&2) lbits|=BIT_B;
                if ((y-1)&4) lbits|=BIT_C;
                int mask=(1<<(8-BITPLANE_CNT+plane));         //bitmask for pixel data in input for this bitplane
                uint8_t *p=bitplane[backbuf_id][plane] + y * 32 * (NULLFRAMES+1);         //bitplane location to write to
                for (int fx=0; fx<32; fx++) {
                    int x = fx^2;   //Apply correction. this fixes dma byte stream order
                    int v = lbits;
                    //Do not show image while the line bits are changing
                    //Don't display for the first cycle to remove line bleed
                    if (x<1 || x>=brightness) v|= BIT_OE;
                    if (x==31) v|= BIT_LAT;         //latch on last bit...
                    Color c1;
                    int yreal = y;
                    int xreal = 31-x;
                    if (hub75_framebuffer) {
                            c1 = hub75_framebuffer[yreal*CONFIG_HUB75_WIDTH+xreal];
                    } else {
                            c1.value = 0; //If no framebuffer available display is off.
                    }

                    if (valToPwm(c1.RGB[3]) & mask) {
                        v |= BIT_R1;
                        total_intensity += mask;

                    }
                    if (valToPwm(c1.RGB[2]) & mask) {
                        v |= BIT_G1;
                        total_intensity += mask;
                    }
                    if (valToPwm(c1.RGB[1]) & mask) {
                        v |= BIT_B1;
                        total_intensity += mask;
                    }


                    //Save the calculated value to the bitplane memory
                    *p++=v;
                }

                lbits=0;  //Precalculate line bits of the *current* line
                if ((y)&1) lbits|=BIT_A;
                if ((y)&2) lbits|=BIT_B;
                if ((y)&4) lbits|=BIT_C;
                for (int n=0; n < NULLFRAMES; n++) {
                    for (int fx = 0; fx < 32; fx++) {
                        int x = fx ^ 2;  // Apply correction. this fixes dma byte stream order
                        int v = lbits;

                        if (x>=brightness) { v|= BIT_OE; }

                        if ((n == NULLFRAMES-1) && x == 31) {
                          v |= BIT_LAT;  // latch on last bit...
                        }

                        // Save the calculated value to the bitplane memory
                        *p++ = v;
                    }
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

        ledc_timer_config_t ledc_timer = {
            .duty_resolution = LEDC_TIMER_8_BIT, // resolution of PWM duty
            .freq_hz = 20000,                      // frequency of PWM signal
            .speed_mode = LEDC_HIGH_SPEED_MODE,           // timer mode
            .timer_num = LEDC_TIMER_0,            // timer index
        };
        ledc_timer_config(&ledc_timer);
        ledc_channel_config_t led_power_dimmer = {
            .channel    = 0,
            .duty       = 0,
            .gpio_num   = 12,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .hpoint     = 0,
            .timer_sel  = LEDC_TIMER_0
        };
        ledc_channel_config(&led_power_dimmer);

        bool usb_connected = true;
        TickType_t lastUsbCheck = 0;
        uint8_t duty = 255;
        uint32_t deduced_current = 0;

	while(driver_hub75_active) {
		if(compositor_status()) composite();
		render16();

#ifdef CONFIG_HUB75_REGULATE_VLED
                if (xTaskGetTickCount() - lastUsbCheck > 1000/portTICK_PERIOD_MS) {
                    lastUsbCheck = xTaskGetTickCount();
                    int vusb = adc1_get_raw(ADC1_CHANNEL_6) * 2; // in raw dimensionless units
//                    printf("vusb: %d\n", vusb);
                    usb_connected = vusb > 1000; // arbitrary cutoff (normal values 1500-5000 when connected)
                }

                if (usb_connected) {
                    uint32_t intensity        = total_intensity;
                    uint8_t milliamps_per_led = 35;
                    deduced_current =
                        (uint32_t)((intensity / 255.0) * milliamps_per_led *
                                   ((brightness - 2) / 32.0) / 8);  // in milliamps

                    // Correct for blank frames. Example: 1 data frame + 3 blank frames, of
                    // which 1 blanks the row, actual current is <deduced> * 3/4.
                    if (NULLFRAMES) {
//                      deduced_current *= NULLFRAMES;
//                      deduced_current /= NULLFRAMES+1;
                      deduced_current /= 3; // this silly heuristic works
                    }

                    if (deduced_current <= LOWER) {
                      duty = PCT_MIN;
                    } else if (deduced_current <= MID) {
                      duty = (int)((deduced_current - LOWER) / ((float)MID - LOWER) *
                                   (PCT_MID - PCT_MIN)) +
                             PCT_MIN;
                    } else if (deduced_current <= UPPER) {
                      duty = (int)((deduced_current - MID) / ((float)UPPER - MID) * (255 - PCT_MID)) +
                             PCT_MID;
                    }
                } else {
                  duty = 255;
                }

                if (duty != 255) {
//                    printf("%d %d\n", deduced_current, duty);
                }
                ledc_set_duty(led_power_dimmer.speed_mode, led_power_dimmer.channel, duty);
                ledc_update_duty(led_power_dimmer.speed_mode, led_power_dimmer.channel);
#endif // CONFIG_HUB75_REGULATE_VLED

		vTaskDelayUntil( &xLastWakeTime, 1.0 / framerate * 1000 / portTICK_PERIOD_MS );
	}
	vTaskDelete( NULL );
}

Color* getFrameBuffer()
{
        return hub75_framebuffer;
}

esp_err_t driver_hub75_init(void)
{
	static bool driver_hub75_init_done = false;
	if (driver_hub75_init_done) return ESP_OK;
	ESP_LOGD(TAG, "init called");

#ifdef CONFIG_HUB75_REGULATE_VLED
        // Initialise the vUSB sensor so we can sense when vLED correction is needed
        adc1_config_width(ADC_WIDTH_BIT_10);
        adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
#endif

	//Change to set the global brightness of the display, range 1-63
	//Warning when set too high: Do not look into LEDs with remaining eye.
	#ifndef CONFIG_DRIVER_FRAMEBUFFER_ENABLE
	hub75_framebuffer = calloc(HUB75_BUFFER_SIZE, sizeof(uint8_t));
	#endif

	for (int i=0; i<BITPLANE_CNT; i++) {
		for (int j=0; j<2; j++) {
			bitplane[j][i] = (uint8_t *) heap_caps_calloc(BITPLANE_SZ, sizeof(uint8_t), MALLOC_CAP_DMA);
			if (!bitplane[j][i]) {
				ESP_LOGE(TAG, "Can't allocate bitplane memory");
				return ESP_FAIL;
			}
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
	compositor_setBuffer(getFrameBuffer());
	#endif

	driver_hub75_active = true;

	xTaskCreatePinnedToCore(
		&displayTask, /* Task function. */
		"display",    /* String with name of task. */
		4096,         /* Stack size in words. */
		NULL,         /* Parameter passed as input of the task */
		1,            /* Priority of the task. (Lower = more important) */
		NULL,         /* Task handle. */
		1             /* Core ID */
	);

	driver_hub75_init_done = true;
	ESP_LOGD(TAG, "init done");
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
